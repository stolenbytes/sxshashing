#!/usr/bin/env	python
#
# x86_microsoft.windows.common-controls_6595b64144ccf1df_5.82.9600.16384_none_7c55c866aa0c3ff0
#
# order of building manifest is:
# name -> culture -> type -> version -> publicKeytoken -> processorArchitecture
#
# and final folder layout is created as:
#
# processorArchitecture_name_publicKeytoken_version_culture_hash <generated>
#
#

import  pefile
from    xml.etree import ElementTree
import  sys

def	hash_char(hash, x):
	hash = (hash * 0x1003F) & 0xffffffff;
	hash = (hash + ord(x)) & 0xffffffff;
	return hash;

def	hashlower(data):
	buff = data.lower();
	
	hash = [0,0,0,0];
	
	for idx, x in enumerate(buff):
		idx = idx % 4;
		hash[idx] = hash_char(hash[idx], x);

	hash = hash[0] * 0x1E5FFFFFD27 + hash[1] * 0xFFFFFFDC00000051 + hash[2] * 0x1FFFFFFF7 + hash[3];
	hash = hash & 0xffffffffffffffff;
	return hash;


if len(sys.argv) < 2:
        print("sxshashing.py <pefile> [fakeversion]");
        exit(0);
        
pe = pefile.PE(sys.argv[1]);

rt_manifest_idx = [entry.id for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries].index(pefile.RESOURCE_TYPE["RT_MANIFEST"])

data = "";

rtmanifest = pe.DIRECTORY_ENTRY_RESOURCE.entries[rt_manifest_idx]
for entry in rtmanifest.directory.entries:
        data_rva = entry.directory.entries[0].data.struct.OffsetToData;
        size     =  entry.directory.entries[0].data.struct.Size;
        data = pe.get_memory_mapped_image()[data_rva:data_rva+size]
        break;
        

root = ElementTree.fromstring(data);
dependency = None;
dependentAssembly = None;
assemblyIndentity = None;

for child in root:
        if "dependency" in child.tag:
                dependency = child;
                break;

for child in dependency:
        if "dependentAssembly" in child.tag:
                dependentAssembly = child;
                break;

for child in  dependentAssembly:
        if "assemblyIdentity" in child.tag:
                assemblyIdentity = child;
                break;            


name    = assemblyIdentity.attrib["name"].lower();
culture = assemblyIdentity.attrib["language"].lower();
if culture == "*":
        culture = "none";
typee   = assemblyIdentity.attrib["type"].lower();
version = assemblyIdentity.attrib["version"].lower();
publicKeyToken = assemblyIdentity.attrib["publicKeyToken"].lower();
processorArchitecture = assemblyIdentity.attrib["processorArchitecture"].lower();
if processorArchitecture == "*":
        if pe.FILE_HEADER.Machine == pefile.MACHINE_TYPE["IMAGE_FILE_MACHINE_AMD64"]:
                processorArchitecture = "amd64";
        elif pe.FILE_HEADER.Machine == pefile.MACHINE_TYPE["IMAGE_FILE_MACHINE_I386"]:
                processorArchitecture = "x86";
data0 = ["name", name];
data1 = ["culture", culture];
data2 = ["type", typee];
data3 = ["version", version];
data4 = ["publicKeytoken", publicKeyToken];
data5 = ["processorArchitecture", processorArchitecture];


final_hash = 0;



data = [data0, data1, data2, data3,data4, data5];
dict = {};

for x in data:
        dict[x[0]] = x[1];

versionhash = 0;
doversion   = 0;
final_hash = 0;
for x in data: 
        if x[1] == "none": continue;
        hash_attr  = hashlower(x[0]);
        hash_val   = hashlower(x[1]);
        
        both_hashes = hash_val + 0x1FFFFFFF7 * hash_attr;
        final_hash = both_hashes + 0x1FFFFFFF7 * final_hash;
	if x[0] != "version":
		versionhash = both_hashes + 0x1FFFFFFF7 * versionhash;
		
#build string:
fullname = [];
fullname.append(dict["processorArchitecture"]);
fullname.append(dict["name"]);
fullname.append(dict["publicKeytoken"]);
fullname.append(dict["version"]);
fullname.append(dict["culture"]);
fullname.append("%.016x" % (final_hash & 0xffffffffffffffff)); 
#fullname.append(hex(final_hash & 0xffffffffffffffff)[2:-1]);
print("Full WinSXS path:");
print("_".join(fullname));

#build string to query Winners registry key...

fullname = [];
fullname.append(dict["processorArchitecture"]);
fullname.append(dict["name"]);
fullname.append(dict["publicKeytoken"]);
fullname.append(dict["culture"]);
fullname.append("%.016x" % (versionhash & 0xffffffffffffffff)); 
#fullname.append(hex(versionhash & 0xffffffffffffffff)[2:-1]);

print("Winners key:");
print("_".join(fullname));
#build full string version:
               
data0 = ["name", name];
data1 = ["culture", culture];
data2 = ["type", typee];
if len(sys.argv) >= 3:
        data3 = ["version", sys.argv[2]];
else:
        data3 = ["version", "6.0.65535.65535"];
data4 = ["publicKeytoken", publicKeyToken];
data5 = ["processorArchitecture", processorArchitecture];


final_hash = 0;



data = [data0, data1, data2, data3,data4, data5];
dict = {};

for x in data:
        dict[x[0]] = x[1];

versionhash = 0;
doversion   = 0;
final_hash = 0;
for x in data: 
        if x[1] == "none": continue;
        hash_attr  = hashlower(x[0]);
        hash_val   = hashlower(x[1]);
        
        both_hashes = hash_val + 0x1FFFFFFF7 * hash_attr;
        final_hash = both_hashes + 0x1FFFFFFF7 * final_hash;
	if x[0] != "version":
		versionhash = both_hashes + 0x1FFFFFFF7 * versionhash;
		
#build string:
fullname = [];
fullname.append(dict["processorArchitecture"]);
fullname.append(dict["name"]);
fullname.append(dict["publicKeytoken"]);
fullname.append(dict["version"]);
fullname.append(dict["culture"]);
fullname.append("%.016x" % (final_hash & 0xffffffffffffffff)); 
print("Full WinSXS path fake version:");
print("_".join(fullname));        
