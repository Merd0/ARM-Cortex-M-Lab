"""Negative controls for the documentation gate; no firmware or repo edits."""
import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

SPEC = importlib.util.spec_from_file_location("check_docs", Path(__file__).with_name("check-docs.py"))
GATE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GATE)

SOURCE = """# Ders

## Deney

`run --preset debug` CHECK_FRAME=PASS; PC=0x1000; budget 32.

```text
[CHECK_FRAME] PASS
[PC] 0x1000
```

[Deney](#deney)
"""
ENGLISH = SOURCE.replace("# Ders", "# Lesson").replace("## Deney", "## Experiment").replace(
    "[Deney](#deney)", "[Experiment](#experiment)")


class DocumentationGateTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="arm-doc-gate-")
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name).resolve()
        (self.root / "docs").mkdir()
        self.manifest = {
            "baseline": "fixture",
            "documents": [{"original": "README.md", "english": "README.md",
                           "turkish": "README.tr.md", "state": "translated",
                           "source_sha256": hashlib.sha256(SOURCE.encode()).hexdigest()}],
        }
        self.write("README.md", "**Language:** English | [Türkçe](README.tr.md)\n\n" + ENGLISH)
        self.write("README.tr.md", "**Dil:** [English](README.md) | Türkçe\n\n" + SOURCE)

    def write(self, path, text):
        (self.root / path).write_text(text, encoding="utf-8", newline="\n")

    def change(self, path, old, new):
        text = (self.root / path).read_text(encoding="utf-8")
        self.assertIn(old, text)
        self.write(path, text.replace(old, new))

    def run_gate(self, *args):
        self.write("docs/documentation-coverage.json", json.dumps(self.manifest))
        output = io.StringIO()
        with patch.object(GATE, "ROOT", self.root), \
                patch.object(GATE, "source_at", return_value=SOURCE.encode()), \
                patch.object(GATE, "markdown_inventory", return_value={"README.md", "README.tr.md"}), \
                patch("sys.argv", ["check-docs.py", *args]), contextlib.redirect_stdout(output):
            failed = GATE.main()
        return failed, output.getvalue()

    def rejects(self, expected):
        failed, output = self.run_gate()
        self.assertTrue(failed, output)
        self.assertIn(expected, output)

    def test_valid_complete_pair(self):
        self.assertFalse(self.run_gate()[0])

    def test_pending_cannot_pass_completion(self):
        self.manifest["documents"][0]["state"] = "pending"
        self.rejects("pending translation")
        failed, output = self.run_gate("--allow-pending")
        self.assertFalse(failed, output)
        self.assertIn("PARTIAL BATCH CHECK ONLY", output)

    def test_turkish_rewrite_is_rejected_even_when_pending(self):
        self.manifest["documents"][0]["state"] = "pending"
        self.change("README.tr.md", "budget 32", "budget 33")
        failed, output = self.run_gate("--allow-pending")
        self.assertTrue(failed)
        self.assertIn("Turkish source changed", output)

    def test_missing_counterpart(self):
        (self.root / "README.tr.md").unlink()
        self.rejects("missing preserved Turkish source")

    def test_wrong_switch(self):
        self.change("README.md", "[Türkçe](README.tr.md)", "[Türkçe](missing.tr.md)")
        self.rejects("invalid language switch")

    def test_observed_output_cannot_change(self):
        self.change("README.md", "[CHECK_FRAME] PASS", "[CHECK_FRAME] FAIL")
        self.rejects("code/output block mismatch")

    def test_command_cannot_change(self):
        self.change("README.md", "`run --preset debug`", "`run --preset release`")
        self.rejects("inline command/identifier mismatch")

    def test_hex_value_cannot_change(self):
        self.change("README.md", "PC=0x1000", "PC=0x2000")
        self.rejects("CHECK/hex value mismatch")

    def test_numeric_observation_cannot_change(self):
        self.change("README.md", "budget 32", "budget 31")
        self.rejects("numerical observation mismatch")

    def test_section_cannot_disappear(self):
        self.change("README.md", "## Experiment\n", "")
        self.rejects("heading structure mismatch")

    def test_missing_relative_file(self):
        self.change("README.md", "(#experiment)", "(missing.md)")
        self.rejects("broken link")

    def test_missing_heading_anchor(self):
        self.change("README.md", "(#experiment)", "(#not-a-heading)")
        self.rejects("broken heading anchor")

    def test_turkish_navigation_cannot_open_english_lesson(self):
        self.change("README.tr.md", "[Deney](#deney)", "[Deney](README.md#experiment)")
        self.rejects("cross-language navigation")

    def test_source_hash_is_checked(self):
        self.manifest["documents"][0]["source_sha256"] = "0" * 64
        self.rejects("source digest mismatch")


if __name__ == "__main__":
    unittest.main()
