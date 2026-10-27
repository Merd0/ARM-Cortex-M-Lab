#!/usr/bin/env python3
"""Critical GDB observations for deferred work, context switching and security."""
import argparse,re,shutil,socket,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def assertion(expression,name):
    return 'if !('+expression+')\n  echo FAIL: '+name+'\\n\n  quit 1\nend\n'

def debugger(elf,evidence,label,commands,ns=None):
    gdb=shutil.which("arm-none-eabi-gdb") or shutil.which("gdb-multiarch")
    assert gdb,"Arm GDB required"
    with socket.socket() as sock:
        sock.bind(("127.0.0.1",0));port=sock.getsockname()[1]
    extra=["-device","loader,file="+str(ns)] if ns else []
    script=evidence/(label+".gdb")
    text="set pagination off\nset confirm off\nset tcp connect-timeout 10\nset tcp auto-retry on\n"
    if ns: text+='add-symbol-file "'+ns.as_posix()+'"\n'
    text+="target remote 127.0.0.1:"+str(port)+"\n"+commands
    text+="echo INTEGRATION_GDB_PASS\\n\ndetach\nquit\n"
    script.write_text(text,encoding="utf-8")
    with (evidence/(label+"-gdb-uart.txt")).open("w") as output:
        q=subprocess.Popen(["qemu-system-arm","-M","mps2-an505","-cpu","cortex-m33",
            "-nographic","-monitor","none","-serial","stdio","-kernel",str(elf),
            *extra,"-S","-gdb","tcp:127.0.0.1:"+str(port)],stdout=output,stderr=subprocess.STDOUT)
        try:
            try:
                run=subprocess.run([gdb,"-q","-nx","-batch",str(elf),"-x",str(script)],
                                   stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=25,cwd=ROOT)
            except subprocess.TimeoutExpired as error:
                partial=error.stdout or b""
                (evidence/(label+"-gdb.txt")).write_bytes(partial if isinstance(partial,bytes) else partial.encode())
                raise AssertionError(label+": GDB timeout") from error
            (evidence/(label+"-gdb.txt")).write_text(run.stdout,encoding="utf-8")
            assert run.returncode==0 and "INTEGRATION_GDB_PASS" in run.stdout,run.stdout
            print("PASS:",label,"GDB",flush=True)
            return run.stdout
        finally:
            if q.poll() is None:
                q.terminate()
                try: q.wait(timeout=3)
                except subprocess.TimeoutExpired: q.kill();q.wait()

def frame_decoder(elf,evidence):
    # Synthetic RAM fixtures, not claims of real fault injection. Stop before main.
    cmd='tbreak main\ncontinue\nsource scripts/gdb/cortex-m.gdb\n'
    cmd+='set $cm_ram_start=0x30006000\nset $cm_ram_end=0x30006100\n'
    cmd+='set {unsigned int}0x30006018=0x10000100\nset {unsigned int}0x3000601c=0x01000000\n'
    for ret in ('0xFFFFFFFD','0xFFFFFFF9','0xFFFFFFBC'):
        cmd+='cm-frame 0x30006000 '+ret+'\n'+assertion('$cm_ok==1','valid basic frame')
    cmd+='set {unsigned int}0x3000601c=0x10000100\nset {unsigned int}0x30006020=0x01000000\n'
    cmd+='cm-frame 0x30006004 0xFFFFFFFD\n'+assertion('$cm_ok==0','reject four-byte-only alignment')
    cmd+='set {unsigned int}0x3000601c=0x01000200\nset $cm_ram_end=0x30006020\n'
    cmd+='cm-frame 0x30006000 0xFFFFFFFD\n'+assertion('$cm_ok==0','reject missing padding')
    cmd+='set $cm_ram_end=0x30006024\ncm-frame 0x30006000 0xFFFFFFFD\n'
    cmd+=assertion('$cm_ok==1','accept bounded padding')
    cmd+='set {unsigned int}0x3000601c=0x0100000f\ncm-frame 0x30006000 0xFFFFFFFD\n'
    cmd+=assertion('$cm_ok==0','reject Thread token with Handler IPSR')
    cmd+='cm-frame 0x30006000 0xFFFFFFF1\n'+assertion('$cm_ok==1','valid Handler MSP')
    cmd+='set {unsigned int}0x3000601c=0x01000000\ncm-frame 0x30006000 0xFFFFFFF1\n'
    cmd+=assertion('$cm_ok==0','reject Handler token with Thread IPSR')
    debugger(elf,evidence,'frame-decoder-boundaries',cmd)

def gateway_entry(elf,ns,evidence,entry,offset=0):
    symbols=subprocess.check_output(["arm-none-eabi-nm",str(ns)],text=True)
    match=re.search(r"^([0-9a-f]+) A "+entry+r"$",symbols,re.M)
    assert match,"imported NSC gateway must exist"
    address=(int(match[1],16)&~1)+offset
    # Synthetic interrupted PC at SG, before the security-state transition.
    # Use a real Thread/PSP hardware frame captured at SysTick entry; no special
    # register writes or privileged debugger access from unprivileged Thread.
    cmd='tbreak *SysTick_Handler if $lr==0xFFFFFFBC\ncontinue\n'
    cmd+=assertion('$lr==0xFFFFFFBC','NS Thread basic frame')
    cmd+='set $entry_sp=$psp_ns\nset $resume_sp=$entry_sp+32+((*(unsigned int*)($entry_sp+28)&512)!=0)*4\n'
    cmd+='set {unsigned int}($entry_sp+24)='+hex(address)+'\n'
    cmd+='break *l15_panic\ntbreak *'+hex(address)+' if $psp_ns==$resume_sp\ncontinue\n'
    if offset:
        cmd+=assertion('$pc==((unsigned int)&l15_panic & ~1)','reject Secure instruction after SG')
    else:
        cmd+=assertion('$pc=='+hex(address)+' && ($xpsr & 511)==0','resume NSC entry without scheduler panic')
        cmd+=assertion('$control_ns==3 && $psp_ns==$resume_sp','unprivileged NS task restored')
        cmd+='stepi\n'
        cmd+=assertion('$pc=='+hex(address+4)+' && ($lr & 1)==0 && $control==$control_s',
                       'SG executes and enters Secure state')
    debugger(elf,evidence,'gateway-entry-'+entry+('-invalid' if offset else ''),cmd,ns)

def context(elf,evidence,label,ret,ns=None):
    image=ns or elf
    disasm=subprocess.check_output(["arm-none-eabi-objdump","-d","--disassemble=PendSV_Handler",str(image)],text=True)
    match=re.search(r"^\s*([0-9a-f]+):.*\bmsr\s+PRIMASK,",disasm,re.M|re.I)
    assert match,"restore boundary must exist"
    restore=int(match[1],16)
    bank="_ns" if ns else ""
    commands="tbreak *l15_schedule\ncontinue\n"
    commands+=assertion("($xpsr & 511)==14", "PendSV handler")
    commands+=assertion("g_l15_current->sp[8]=="+hex(ret),"saved EXC_RETURN")
    commands+=assertion("((unsigned int)g_l15_current->sp+36)==$psp"+bank,"saved PSP position")
    commands+=assertion("((unsigned int)g_l15_current->sp)>=(unsigned int)g_l15_current->low","lower bound")
    commands+=assertion("((unsigned int)g_l15_current->sp+68)<=(unsigned int)g_l15_current->high","upper bound")
    commands+=assertion("($msp"+bank+" & 7)==0","public C stack alignment")
    for reg in range(4,12):
        commands+=assertion("g_l15_current->sp["+str(reg-4)+"]==$r"+str(reg),"saved R"+str(reg))
    commands+='printf "SAVED_CONTEXT=%08x PSP=%08x MSP=%08x\\n",g_l15_current->sp[8],$psp'+bank+',$msp'+bank+'\n'
    commands+="tbreak *"+hex(restore)+"\ncontinue\n"
    commands+=assertion("$lr=="+hex(ret),"restored EXC_RETURN")
    commands+=assertion("$psp"+bank+"==((unsigned int)g_l15_current->sp+36)","restored PSP")
    commands+=assertion("$psplim"+bank+"==(unsigned int)g_l15_current->low","task limit")
    for reg in range(4,12):
        commands+=assertion("g_l15_current->sp["+str(reg-4)+"]==$r"+str(reg),"restored R"+str(reg))
    debugger(elf,evidence,label,commands,ns)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset",choices=("debug","release","minsize"),default="debug")
    args=parser.parse_args()
    build=ROOT/"build"/args.preset;evidence=build/"system-evidence";evidence.mkdir(parents=True,exist_ok=True)
    frame_decoder(build/"projects/fault-analyzer/project-fault-cases.elf",evidence)
    l14=build/"lessons/14-systick-pendsv/14-systick-pendsv.elf"
    cmd="tbreak *PendSV_Handler\ncontinue\n"
    cmd+=assertion("($xpsr & 511)==14 && g_l14_phase==1 && g_l14_inside_tick==0","deferred PendSV")
    cmd+=assertion("g_l14_events[0]==1 && g_l14_events[1]==2","SysTick completion precedes PendSV")
    debugger(l14,evidence,"lesson14-deferred",cmd)
    context(build/"lessons/15-mini-kernel/15-mini-kernel.elf",evidence,"lesson15-context",0xFFFFFFFD)
    folder=build/"final-project/secure-real-time-sensor-node"
    for entry in ('l16_report','l16_service'):
        gateway_entry(folder/'node-bad-svc-secure.elf',folder/'node-bad-svc-nonsecure.elf',evidence,entry)
    gateway_entry(folder/'node-bad-svc-secure.elf',folder/'node-bad-svc-nonsecure.elf',evidence,'l16_report',4)
    s=folder/"node-normal-secure.elf";n=folder/"node-normal-nonsecure.elf"
    context(s,evidence,"node-context",0xFFFFFFBC,n)
    cmd="tbreak *l16_transform\ncontinue\nset $fp_state_before=$control_s & 12\nset $transform_return=$lr & ~1\n"
    cmd+="tbreak *node_calibrate\ncontinue\nset $argument=$r0\nset $resume=$lr & ~1\nset $fpscr_controls=$fpscr & 0x07c00000\n"
    cmd+=assertion("$argument<=15","bounded Secure argument")
    cmd+=assertion("(*(unsigned int*)0xE000ED88 & 0x00f00000)==0x00f00000","Secure FP permission")
    cmd+="tbreak *$resume\ncontinue\n"
    cmd+=assertion("$r0==($argument*3)/2","DSP FP calibration result")
    cmd+=assertion("($fpscr & 0x07c00000)==$fpscr_controls","FPSCR controls preserved")
    cmd+=assertion("$s0==0.0 && $s1==0.0","FP leaf payload erased")
    cmd+="tbreak *$transform_return\ncontinue\n"
    cmd+=assertion("($control_s & 12)==$fp_state_before","FPCA SFPA restored before gateway return")
    debugger(s,evidence,"node-secure-processing",cmd,n)
    for mode in ("kernel-write","cross-task","stack-limit"):
        s=folder/("node-"+mode+"-secure.elf");n=folder/("node-"+mode+"-nonsecure.elf")
        cmd="tbreak *node_fault\ncontinue\n"
        cmd+=assertion("$r1==0xFFFFFFBC","fault EXC_RETURN")
        if mode=="stack-limit":
            cmd+=assertion("(*(unsigned int*)0xE000ED28 & 0x100000)!=0","real STKOF")
        else:
            cmd+=assertion("(*(unsigned int*)0xE000ED28)==0x82","precise MPU denial")
            cmd+=assertion("(*(unsigned int*)0xE000ED34)==attempted_address","valid MMFAR")
        cmd+=assertion("(*(unsigned int*)0xE000ED2C)==0","no HardFault escalation")
        cmd+=assertion("$msp_ns>0x20007000 && $msp_ns<=0x20008000","independent kernel MSP")
        cmd+='source scripts/gdb/cortex-m.gdb\ncm-mode\ncm-fault\n'
        debugger(s,evidence,"node-"+mode,cmd,n)
    s=folder/"node-secure-read-secure.elf";n=folder/"node-secure-read-nonsecure.elf"
    cmd="tbreak *SecureFault_Handler\ncontinue\n"
    cmd+=assertion("($xpsr & 511)==7","SecureFault handler")
    cmd+=assertion("(*(unsigned int*)0xE000EDE4 & ~0x40)==8","AUVIOL")
    cmd+=assertion("(*(unsigned int*)0xE000ED2C)==0","no escalation")
    cmd+=assertion("$msp_s>0x30003000 && $msp_s<=0x30004000","Secure MSP")
    cmd+='printf "SECUREFAULT_LR=%08x SFSR=%08x\\n",$lr,*(unsigned int*)0xE000EDE4\n'
    debugger(s,evidence,"node-secure-read",cmd,n)
    # Controlled NS SysTick during a rejected Thread-origin gateway call.
    s=folder/"node-bad-svc-secure.elf";n=folder/"node-bad-svc-nonsecure.elf"
    disasm=subprocess.check_output(["arm-none-eabi-objdump","-d","--disassemble=PendSV_Handler",str(n)],text=True)
    exits=re.findall(r"^\s*([0-9a-f]+):.*\bbx\s+lr\b",disasm,re.M)
    assert len(exits)==2,"basic restore and Secure defer return boundaries"
    cmd="tbreak *PendSV_Handler if ($lr & 0x40)!=0\ncontinue\n"
    cmd+=assertion("($lr & 0xFFFFFFC3)==0xFFFFFFC0 && ($lr & 8)!=0","Secure Thread owns interrupted context")
    cmd+="set $token=$lr\nset $owner=g_l15_current\nset $saved_sp=g_l15_current->sp\nset $old_psp=$psp_ns\nset $old_limit=$psplim_ns\nset $deferred=g_l15_secure_deferred\n"
    cmd+="tbreak *0x"+exits[-1]+"\ncontinue\n"
    cmd+=assertion("$lr==$token && g_l15_current==$owner && g_l15_current->sp==$saved_sp","no switch or token rewrite")
    cmd+=assertion("$psp_ns==$old_psp && $psplim_ns==$old_limit","NS task stack untouched")
    cmd+=assertion("g_l15_secure_deferred==$deferred+1","deferred counter")
    # Pending ticks can tail-chain at BX LR; observe a stable Secure continuation.
    cmd+="tbreak *node_gateway_resumed\ncontinue\n"
    cmd+=assertion("$pc>=0x10000000 && $pc<0x10071000 && ($xpsr & 511)==0 && g_l15_current==$owner","resume original Secure Thread")
    debugger(s,evidence,"node-secure-defer",cmd,n)

if __name__=="__main__":
    main()
