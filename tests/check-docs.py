"""Check bilingual documentation against its protected Turkish source.

The default gate requires complete coverage. --allow-pending is only for local
translation batches; it does not certify repository-wide completion. These
checks detect structural damage, not semantic translation errors.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parent.parent
FENCES = re.compile(r"^(`{3,}|~{3,})[^\n]*\n.*?^\1[^\n]*(?:\n|$)", re.M | re.S)
LINK = re.compile(r"\[([^\]]*)\]\(([^)]+)\)")
SWITCH = re.compile(r"^\*\*(?:Dil / Language|Language|Dil):\*\*[^\n]*\n\n")


def source_at(revision, path):
    return subprocess.check_output(["git", "show", f"{revision}:{path}"], cwd=ROOT)


def markdown_inventory():
    output = subprocess.check_output(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "--", "*.md"],
        cwd=ROOT, text=True)
    return set(output.splitlines())


def preserved_prose(text):
    """Ignore only the switch and navigation destinations, never link labels."""
    text = SWITCH.sub("", text)
    parts = []
    start = 0
    for block in FENCES.finditer(text):
        parts.append(LINK.sub(lambda m: "[" + m[1] + "]", text[start:block.start()]))
        parts.append(block[0])
        start = block.end()
    parts.append(LINK.sub(lambda m: "[" + m[1] + "]", text[start:]))
    return "".join(parts)


def block_contents(text):
    return [m[0] for m in FENCES.finditer(text)]


def prose(text):
    return FENCES.sub("", SWITCH.sub("", text))


def comparison_text(path, text, row):
    """Only the pre-existing portfolio landing has an additional source guide."""
    if row.get("comparison_scope") is None:
        return text
    start = "<!-- i18n-original-guide:start -->"
    end = "<!-- i18n-original-guide:end -->"
    if path != "README.md" or row["comparison_scope"] != "original-guide":
        raise ValueError("unsupported comparison scope")
    if text.count(start) != 1 or text.count(end) != 1 or text.index(start) >= text.index(end):
        raise ValueError("missing/duplicate original learning guide markers")
    return text.split(start)[1].split(end)[0].strip() + "\n"


def sections(text):
    """Heading sequence and bodies, independent of translated heading wording."""
    text = prose(text)
    headings = list(re.finditer(r"^(#{1,6}) (.+)$", text, re.M))
    return [(len(h[1]), text[h.start():headings[i + 1].start() if i + 1 < len(headings) else len(text)])
            for i, h in enumerate(headings)]


def heading_ids(text):
    counts = Counter()
    result = set(re.findall(r'<a\s+(?:id|name)=[\'"]([^\'"]+)', text))
    for match in re.finditer(r"^#{1,6}\s+(.+?)\s*#*\s*$", FENCES.sub("", text), re.M):
        title = LINK.sub(lambda m: m[1], match[1])
        title = re.sub(r"<[^>]*>", "", title).lower()
        slug = re.sub(r"[^\w\- ]", "", title).replace(" ", "-")
        suffix = f"-{counts[slug]}" if counts[slug] else ""
        counts[slug] += 1
        result.add(slug + suffix)
    return result


def navigation_errors(path, text, paired_paths):
    errors = []
    for match in LINK.finditer(FENCES.sub("", text)):
        target = match[2]
        parsed = urlsplit(target)
        if parsed.scheme or parsed.netloc:
            continue
        resolved = (ROOT / path).parent / unquote(parsed.path) if parsed.path else ROOT / path
        resolved = resolved.resolve()
        if not resolved.exists():
            errors.append(f"broken link: {path} -> {target}")
            continue
        if resolved.is_dir():
            resolved = resolved / "README.md"
        if resolved.suffix != ".md" or not resolved.is_file():
            continue
        relative = resolved.relative_to(ROOT).as_posix()
        is_switch = match.start() < text.find("\n")
        if not is_switch and relative in paired_paths:
            if path.endswith(".tr.md") != relative.endswith(".tr.md"):
                errors.append(f"cross-language navigation: {path} -> {target}")
        if parsed.fragment and unquote(parsed.fragment) not in heading_ids(resolved.read_text(encoding="utf-8")):
            errors.append(f"broken heading anchor: {path} -> {target}")
    return errors


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--allow-pending", action="store_true")
    args = parser.parse_args()
    data = json.loads((ROOT / "docs/documentation-coverage.json").read_text(encoding="utf-8"))
    failures = []
    completed = 0
    paths = set()
    paired_paths = {r[k] for r in data["documents"] for k in ("english", "turkish")}
    for path in markdown_inventory():
        if path not in paired_paths and not path.startswith("external/CMSIS/"):
            failures.append(f"uninventoried first-party Markdown: {path}")
    for row in data["documents"]:
        en, tr = row["english"], row["turkish"]
        for path in (en, tr):
            if path in paths:
                failures.append(f"duplicate path: {path}")
            paths.add(path)
        source_bytes = source_at(data["baseline"], row["original"])
        if hashlib.sha256(source_bytes).hexdigest() != row["source_sha256"]:
            failures.append(f"source digest mismatch: {en}")
        source = source_bytes.decode("utf-8").replace("\r\n", "\n")
        if not (ROOT / tr).is_file():
            failures.append(f"missing preserved Turkish source: {tr}")
        elif preserved_prose(source) != preserved_prose((ROOT / tr).read_text(encoding="utf-8")):
            failures.append(f"Turkish source changed beyond navigation: {tr}")
        if (ROOT / tr).is_file():
            failures.extend(navigation_errors(tr, (ROOT / tr).read_text(encoding="utf-8"), paired_paths))
        if row["state"] != "translated":
            if not args.allow_pending:
                failures.append(f"pending translation: {en}")
            continue
        if not all((ROOT / p).is_file() for p in (en, tr)):
            failures.append(f"missing language pair: {en}")
            continue
        english, turkish = [(ROOT / p).read_text(encoding="utf-8") for p in (en, tr)]
        failures.extend(navigation_errors(en, english, paired_paths))
        for path, text, counterpart, label in (
            (en, english, tr, "**Language:** English | [Türkçe]"),
            (tr, turkish, en, "**Dil:** [English]"),
        ):
            expected = label + "(" + Path(counterpart).name + ")"
            if not text.startswith(expected):
                failures.append(f"invalid language switch: {path}")
            if len(SWITCH.sub("", text).strip()) < 20:
                failures.append(f"empty/placeholder document: {path}")
        try:
            english = comparison_text(en, english, row)
        except ValueError as exc:
            failures.append(f"invalid comparison scope: {en}: {exc}")
            continue
        if re.search(r"\{\{BLOCK\d+\}\}", english):
            failures.append(f"unresolved translation block placeholder: {en}")
        source_blocks, english_blocks = block_contents(source), block_contents(english)
        if len(source_blocks) != len(english_blocks):
            failures.append(f"code/output block count mismatch: {en}")
        else:
            for index, (old, new) in enumerate(zip(source_blocks, english_blocks)):
                allowed = row.get("translated_diagrams", {}).get(str(index))
                if old != new and not (allowed and
                        old.startswith(("```text\n", "```mermaid\n")) and
                        hashlib.sha256(new.encode()).hexdigest() == allowed["english_sha256"]):
                    failures.append(f"code/output block mismatch: {en} block {index}")
        sh = Counter(len(m[1]) for m in re.finditer(r"^(#{1,6}) ", prose(source), re.M))
        eh = Counter(len(m[1]) for m in re.finditer(r"^(#{1,6}) ", prose(english), re.M))
        if sh != eh:
            failures.append(f"heading structure mismatch: {en}")
        token_pattern = r"\b(?:CHECK_[A-Z_0-9]+|0x[0-9A-Fa-f]+)\b"
        if Counter(re.findall(token_pattern, source)) != Counter(re.findall(token_pattern, english)):
            failures.append(f"CHECK/hex value mismatch: {en}")
        inline_pattern = r"`([^`\n]+)`"
        if Counter(re.findall(inline_pattern, prose(source))) != Counter(re.findall(inline_pattern, prose(english))):
            failures.append(f"inline command/identifier mismatch: {en}")
        number_pattern = r"(?<![\w])\d+(?:[.,]\d+)*(?![\w])"
        if Counter(re.findall(number_pattern, prose(source))) != Counter(re.findall(number_pattern, prose(english))):
            failures.append(f"numerical observation mismatch: {en}")
        references = lambda text: Counter(m[2] for m in LINK.finditer(text)
                                          if m[2].startswith(("https://", "http://")))
        if references(source) != references(english):
            failures.append(f"official/external reference changed: {en}")
        source_sections, english_sections = sections(source), sections(english)
        if [level for level, _ in source_sections] != [level for level, _ in english_sections]:
            failures.append(f"section hierarchy/order mismatch: {en}")
        elif source_sections:
            for index, ((_, before), (_, after)) in enumerate(zip(source_sections, english_sections), 1):
                for label, pattern in (("inline commands", inline_pattern),
                                       ("CHECK/hex values", token_pattern),
                                       ("numbers", number_pattern)):
                    if Counter(re.findall(pattern, before)) != Counter(re.findall(pattern, after)):
                        failures.append(f"section {index} {label} mismatch: {en}")
                before_body = before.partition("\n")[2].strip()
                after_body = after.partition("\n")[2].strip()
                if len(before_body) > 80 and len(after_body) < len(before_body) * 0.30:
                    failures.append(f"section {index} missing/substantially shortened prose: {en}")
        completed += 1
    print(f"{completed}/{len(data['documents'])} translated pairs checked")
    print("Turkish preservation, source hashes, switches, blocks, headings, CHECK/hex values, navigation/anchors")
    for failure in failures:
        print("FAIL:", failure)
    if args.allow_pending:
        print("PARTIAL BATCH CHECK ONLY; pending pairs are not certified.")
    return bool(failures)


if __name__ == "__main__":
    sys.exit(main())
