#!/usr/bin/env python3
"""Integrated projects/final behavioral gate; each expected CHECK must be present."""
import argparse
from pathlib import Path
import runpy

ROOT = Path(__file__).resolve().parents[1]
verify = runpy.run_path(str(ROOT / "tests/development-smoke.py"))["verify"]
KERNEL = ("CHECK_INITIAL_FRAME","CHECK_TASK_PROGRESS","CHECK_REGISTERS","CHECK_STACK_GUARDS",
          "CHECK_TASK_STACKS","CHECK_PREEMPTION","CHECK_YIELD","CHECK_HANDLER_MSP","CHECK_NO_FAULT")

def projects(build, evidence):
    def run(folder, name, keys):
        verify(build/"projects"/folder/(name+".elf"), evidence, required=keys)
    run("from-reset-to-main","project-reset",
        ("CHECK_DATA","CHECK_BSS","CHECK_ORDER","CHECK_VECTORS","CHECK_VTOR","CHECK_LMA"))
    run("fault-analyzer","project-fault-cases",("CHECK_VALID_PSP","CHECK_VALID_MSP","CHECK_VALID_NS",
        "CHECK_STACK_ERROR_REJECT","CHECK_FP_REJECT","CHECK_CROSS_SECURITY_REJECT","CHECK_BOUNDS_REJECT",
        "CHECK_INVALID_ADDRESS","CHECK_MMFAR"))
    for kind in ("udf","unaligned","bus","mpu","escalated"):
        run("fault-analyzer","project-fault-"+kind,("CHECK_CAUSE","CHECK_FRAME","CHECK_PRIVILEGE","CHECK_CLEAR"))
    for kind in ("normal","denied"):
        keys=("CHECK_UNPRIV_PSP","CHECK_SVC","CHECK_REJECT","CHECK_HANDLER")
        keys+=(("CHECK_NO_FAULT",) if kind=="normal" else ("CHECK_MPU_DENIAL","CHECK_NOT_ESCALATED"))
        run("protected-application","project-protected-"+kind,keys)
    for opt in ("o0","o1","o2","o3","os"):
        run("performance-profiler","project-profile-"+opt,
            ("CHECK_CLASSIFICATION","CHECK_WORKLOAD","CHECK_PROGRESS","CHECK_NO_FAULT"))
    run("mini-kernel","project-kernel",KERNEL+("CHECK_QUEUE",))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset",choices=("debug","release","minsize"),default="debug")
    args=parser.parse_args()
    build=ROOT/"build"/args.preset
    evidence=build/"system-evidence"; evidence.mkdir(parents=True,exist_ok=True)
    projects(build,evidence)

if __name__=="__main__":
    main()
