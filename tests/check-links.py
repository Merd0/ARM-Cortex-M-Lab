import os,re,sys
root=os.path.abspath('.')
md_link=re.compile(r'\[([^\]]*)\]\(([^)]+)\)')
bad=[]; checked=0
for dp,dn,fn in os.walk(root):
    dn[:]=[d for d in dn if d not in ('build','.git','CMSIS')]
    for f in fn:
        if not f.endswith('.md'): continue
        p=os.path.join(dp,f)
        for i,line in enumerate(open(p,encoding='utf-8'),1):
            for text,target in md_link.findall(line):
                t=target.split('#')[0].strip()
                if not t or t.startswith(('http://','https://','mailto:')): continue
                checked+=1
                resolved=os.path.normpath(os.path.join(dp,t))
                if not os.path.exists(resolved):
                    bad.append((os.path.relpath(p,root),i,target))
print(f"checked {checked} relative links")
for b in bad: print("BROKEN:",b[0],"line",b[1],"->",b[2])
sys.exit(1 if bad else 0)
