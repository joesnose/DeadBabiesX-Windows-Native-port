#!/usr/bin/env python3
"""Offline metadata inventory. Does not redistribute firmware or decompiler text."""
from pathlib import Path
import argparse,collections,csv,hashlib,json,re,struct
p=argparse.ArgumentParser();p.add_argument('--capture',type=Path,required=True);p.add_argument('--firmware',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output.mkdir(exist_ok=True,parents=True)
manifest=json.loads((a.capture/'manifest.json').read_text());imports=json.loads((a.capture/'imports.json').read_text())
modules=[]
for name in sorted(imports):
 m=manifest[name];data=(a.capture/f'pspgo_ds4_code_{name}.bin').read_bytes()
 modules.append({'module':name,'text_bytes':m['size'],'capture_bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'export_entries':len(m['exports']),'unique_export_addresses':len({x['address'] for x in m['exports']}),'import_entries':len(imports[name]),'import_libraries':sorted({x['library'] for x in imports[name]})})
(a.output/'module-inventory.json').write_text(json.dumps(modules,indent=2)+'\n')
files=[];extensions=collections.Counter()
for f in sorted(a.firmware.rglob('*')):
 if not f.is_file():continue
 data=f.read_bytes();elf=data[:4]==b'\x7fELF';machine=struct.unpack_from('<H',data,18)[0] if elf and len(data)>=20 else None
 files.append({'path':str(f.relative_to(a.firmware)),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'elf_machine':machine,'dfu_suffix_marker':len(data)>=16 and data[-8:-5]==b'UFD' and data[-5]==16,'xuv_line_marker':bool(re.search(rb'(?m)^@[0-9A-Fa-f]{6,8} [0-9A-Fa-f]{4}\r?$',data))});extensions[f.suffix.lower()]+=1
(a.output/'firmware-file-inventory.json').write_text(json.dumps(files,indent=2)+'\n')
commands=[]
def last_assignment(prefix,name):
 vals=re.findall(r'\b'+re.escape(name)+r'\s*=\s*(0x[0-9a-fA-F]+|[0-9]+)\s*;',prefix)
 return int(vals[-1],0) if vals else None
def plus_two(name):
 m=re.fullmatch(r'local_([0-9a-f]+)',name)
 return 'local_'+format(int(m[1],16)-2,'x') if m else ''
for f in sorted((a.capture/'functions').glob('*.c')):
 text=f.read_text();header=re.search(r'ADDRESS ([0-9a-f]+) NAME (\w+)',text)
 if not header:continue
 address=int(header[1],16)
 if not (manifest['sceBSMan']['base']<=address<manifest['sceBSMan']['base']+manifest['sceBSMan']['size']):continue
 for match in re.finditer(r'FUN_8823c920\(&([A-Za-z_0-9]+),&([A-Za-z_0-9]+),',text):
  req,rsp=match.groups();prefix=text[:match.start()];command=last_assignment(prefix,req)
  if command is None:continue
  response=last_assignment(prefix,rsp)
  commands.append({'function_offset':hex(address-manifest['sceBSMan']['base']),'symbol':header[2],'request':hex(command),'request_payload_bytes_candidate':last_assignment(prefix,plus_two(req)),'response':hex(response) if response is not None else None,'response_data_capacity_candidate':last_assignment(prefix,plus_two(rsp)),'confidence':'literal-assignment candidate; not control-flow proven'})
with (a.output/'command-candidates.csv').open('w',newline='') as f:
 w=csv.DictWriter(f,fieldnames=list(commands[0]));w.writeheader();w.writerows(commands)
summary={'module_count':len(modules),'captured_text_bytes':sum(x['text_bytes'] for x in modules),'decompiled_function_files':len(list((a.capture/'functions').glob('*.c'))),'firmware_file_count':len(files),'firmware_total_bytes':sum(x['bytes'] for x in files),'extensions':dict(extensions),'command_call_candidates':len(commands),'unique_request_candidates':len({x['request'] for x in commands}),'named_radio_image_candidates':[x['path'] for x in files if Path(x['path']).suffix.lower() in ('.dfu','.xuv','.xpv','.xdv','.xhv')],'dfu_suffix_candidates':[x['path'] for x in files if x['dfu_suffix_marker']],'xuv_text_candidates':[x['path'] for x in files if x['xuv_line_marker']],'elf_machine_counts':dict(collections.Counter(str(x['elf_machine']) for x in files if x['elf_machine'] is not None)),'note':'No matching filename is not proof that no embedded/compressed radio image exists.'}
(a.output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary,indent=2))
