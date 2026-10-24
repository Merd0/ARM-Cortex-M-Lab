#!/usr/bin/env python3
"""Static workload code size/instruction count; deliberately no speed ranking."""
import argparse,json,re,subprocess
from pathlib import Path
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument("--preset",default="debug",choices=("debug","release","minsize"))
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]; build=root/"build"/args.preset
result={"scope":"Static emitted code, not executed instruction counts or CPU cycles","workload":[]}
for opt in ("o0","o1","o2","o3","os"):
    elf=build/"projects/performance-profiler"/("project-profile-"+opt+".elf")
    symbols=subprocess.check_output(["arm-none-eabi-nm","-S",str(elf)],text=True)
    match=re.search(r"^[0-9a-fA-F]+ ([0-9a-fA-F]+) T l12_workload$",symbols,re.M)
    assert match,elf
    disasm=subprocess.check_output(["arm-none-eabi-objdump","-d","--disassemble=l12_workload",str(elf)],text=True)
    count=0
    for line in disasm.splitlines():
        m=re.match(r"\s*[0-9a-fA-F]+:\s+(?:[0-9a-fA-F]{4}\s+)+([a-z][a-z0-9.]*)\b",line)
        if m: count+=1
    assert count>0
    result["workload"].append({"optimization":"-O"+opt[1:],"bytes":int(match[1],16),"static_instructions":count})
folder=build/"system-evidence";folder.mkdir(parents=True,exist_ok=True)
(folder/"profile.json").write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
print(json.dumps(result,indent=2))
