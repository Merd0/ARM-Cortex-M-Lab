#!/usr/bin/env python3
"""Integrated projects/final behavioral gate; each expected CHECK must be present."""
import argparse
import re
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
        "CHECK_INVALID_ADDRESS","CHECK_MMFAR","CHECK_ALIGNMENT_REJECT","CHECK_PADDING",
        "CHECK_MODE_REJECT","CHECK_VALID_HANDLER","CHECK_HANDLER_MODE_REJECT",
        "CHECK_PC_REJECT","CHECK_THUMB_REJECT"))
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
    parser.add_argument("--group",choices=("projects","final","all"),default="all")
    args=parser.parse_args()
    build=ROOT/"build"/args.preset
    evidence=build/"system-evidence"; evidence.mkdir(parents=True,exist_ok=True)
    if args.group in ("projects","all"): projects(build,evidence)
    if args.group in ("final","all"): final(build,evidence)

def final(build,evidence):
    folder=build/"final-project/secure-real-time-sensor-node"
    prefix=("CHECK_PARTITION","CHECK_NS_VECTORS")
    secure=("CHECK_SECURE_PROCESSING","CHECK_NS_FPU_DISABLED","CHECK_SECURE_CALLER")
    normal=("CHECK_PIPELINE","CHECK_RESULT","CHECK_TASK_PRIVILEGE","CHECK_REGISTERS",
            "CHECK_STACKS","CHECK_BASIC_CONTEXT","CHECK_SCHEDULING","CHECK_MODEL_BUDGET","CHECK_NO_FAULT")
    for mode in ("normal","kernel-write","cross-task","secure-read","stack-limit","spin","bad-svc"):
        values={}
        if mode=="secure-read":
            keys=prefix+("CHECK_EXPECTED_SECURE_FAULT","CHECK_SECURE_HANDLER","CHECK_NO_ESCALATION")
            values={"SFSR":"0x00000008","SFAR":"INVALID"}
        elif mode in ("kernel-write","cross-task","stack-limit"):
            fault="CHECK_STACK_FAULT_REJECT" if mode=="stack-limit" else "CHECK_EXPECTED_MPU_FAULT"
            keys=prefix+secure+(fault,"CHECK_FAULT_NOT_ESCALATED","CHECK_KERNEL_STACK","CHECK_GUARDS","CHECK_PIPELINE_STARTED")
            values={"EXC_RETURN":"0xFFFFFFBC","HFSR":"0x00000000"}
        else:
            keys=prefix+normal+secure
            if mode=="spin": keys+=("CHECK_SPIN_PREEMPTED",)
            if mode=="bad-svc": keys+=("CHECK_BAD_SVC_REJECT","CHECK_DIAGNOSTIC_POLICY","CHECK_SECURE_CONTEXT_DEFERRED")
            # Independent integer oracle for sliding four-sample average and 1.5 gain.
            samples=[i%16 for i in range(32)]
            expected=sum(((sum(samples[max(0,i-3):i+1])//min(i+1,4))*3)//2 for i in range(32))
            values={"PRODUCED":32,"PROCESSED":32,"LOGGED":32,"FILTERED_SUM":expected,
                    "THREAD_CONTROL":"0x00000003","LAST_SVC_RETURN":"0xFFFFFFBC"}
        text=verify(folder/("node-"+mode+"-secure.elf"),evidence,values=values,required=keys,
               extra=("-device","loader,file="+str(folder/("node-"+mode+"-nonsecure.elf"))))
        if mode in ("normal","spin","bad-svc"):
            assert re.findall(r"^\[TELEMETRY_SEQ\] (\d+)$",text,re.M)==["8","16","24","32"],text
            assert re.findall(r"^\[TELEMETRY_VALUE\] (\d+)$",text,re.M)==["7","19","7","19"],text

if __name__=="__main__":
    main()
