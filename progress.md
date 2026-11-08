dsa mini project 01

# Understanding of Project and How It Should Be Started

I started with implementing the stack class that has nodes in each index, that wasnt difficult, but the last function of snapshot into needed understanding.

Ive written my understanding from line 116 to 122 in comments.

To understand snapshot I first read through what frame means and what a timeline is.

Then I implemented the timeline class, was technically a doubly linked list but data as snapshot.

There were no such difficulties in implementing it, bas record wala func mey aik masla aya tha wo sahi kerlia.

# understanding every struct

variable: isme bas name or uski vale store kerni

frame: hamey aik function call ki state pata chalti hai

jiske andar funcation name ayega, argc tells us ke func me kitne parameters aye hain,

or in argv ki array we have function ke parameters values

locals are the variables jo fucntion ke andar baney and so we have locals ka count. 

returnLine tells ke jab fucntion execute hogya pur to kaha ja ker continue hona in main?

like this hamare pas frame stack hai jisme har fucntion ki call state store hoti hai.

snapshot: live state of pura stack, copy kerke rakh leta, and stack depth is us time per us copy huye huye ki depth

TTDBHeader: it is for .tdbg file , binary debigger file?

we will see the rest when we call them and see kaha use hore hain



# iam now on 0x0 (is the function valid)
why inside loop we use size_t instead of int kyuki we are comparing sizes like container lengths .size() etc to size_t is an unsigned integer type guaranteed to be large enough to represent the size of any possible object.

also like we have 32 bit and 64 bit, to i can adapt itself unke liye

# ON 0x1
why we need source.bin and resolve.bin: to jump over cause we have calls etc yk

resolve.bin has offset is of 8 bytes, string size is of 4 bytes, string is of that many bytes jitna string ka size ho

we have next offset ka formula = offset+8+ 4+ string size

we can later on say is offset per ye record rakha hai

achah iam writing my understanding and explainations comments me bhe sath sath, baki necessary cheeze idher add ker ri

write resolve record me size nai given to wo ham khud calculate ker lete hain

firsrt i simply wrote it like: int32_t size = text.size(), lekin phir because binary file format ke liye size filed int 32 hai, or text.size size t return kerta, so intentionally convert kerne ke liye

Pichli baar write kerte huye hum file format ke liye cast kar rahe thay int 32 mein. Ab hum function return type ke saath comparison ke liye cast kar rahe hain size t me

fread() returns size_t, while size is int32_t.
Convert size to size_t for a type-consistent comparison.


acha so last resolve program wale func mein, jab hamare pas func ho to hamne fun array me stor ekerni uski position and name, or jab call to positon and name in patches

in resolver wb+ means:

w = create/truncate the file

b = binary mode.

+ = allow both reading and writing

Because resolver pehle records write karega, phir wapas seek karke CALL offsets overwrite karega.

writeResolveRecord() humein record ki starting position return karta hai, Hum isi position ko FuncEntry aur PendingPatch mein use karenge.

JAB FUNC ENCOUTNER KEREYIN TO YAD RAKHNA HAI KE : starting byte position of its FUNC record

JAB CALL ENCOUNTE RKERYEIN TO PHIR YAD RAKHNA HAI: IS CALL record ke offset field mein baad mein FUNC ka address likhna hai


# checking resolver chal ra hai ya nai

<img width="1268" height="402" alt="image" src="https://github.com/user-attachments/assets/e1578106-71c5-4f42-ae6f-700349309862" />

<img width="1164" height="250" alt="image" src="https://github.com/user-attachments/assets/af434e98-fac4-48c6-8a94-33773c121238" />

prove cal patched at 63 and func foo starts at 63 aswell

# ON 0x2 (execution)

ab 0x2 mein resolve.bin ki instructions execute kerni hain. 0x1 mein sirf function addresses resolve huye thay, ab variables update honge, function calls hongi and har instruction ke baad snapshot save hoga.



# tokenizeLine()
line ko words mein break kerene ke liye,
MAX_TOKENS also checked 

# buildSnapshot()

isme new Snapshot banate hain and snapshot_into() current callStack ke frames copy ker deta hai.

stackDepth batata hai kitne frames copy huye matlab har instruction ke baad us moment ki state save hojati hai. live stack baad mein change ho bhi jaye to old snapshot same rahega.

# findVariable()

variables do jagah ho sakte hain, argv mein parameters ya locals mein local variables, so findVariable dono arrays mein search karta hai.

agar variable mil jaye to pointer otherwise nullptr return, pointer isliye because original variable ki value update kerni hai yk

# getValue()

operand hamesha number nahi hota like add x y mein y variable ho sakta hai.

so findVariable se  pehle variable dhoondta hai, warna stol() se number invalid ya  out of range error check too


## executeProgram loop

pc is program counter its resolve.bin ka byte number jahan agli instruction rakhi hai.


whats each iteration doin:

1. fseek(pc): file ka pointer us instruction par le jao.
2. readResolveRecord: wo ek record (offset + size + line) parh lo.
3. ftell: ab file ka pointer agle record par hai, ye nextPc hai.
4. tokenizeLine: line ko words mein todo (keyword, naam, params).
5. Keyword ke hisab se kaam karo, phir snapshot record karo.

why we compute next pc call branch se pehle:
call kerke to pointer called func me chala jayega to it should knwo na ke usne wapis kaha ana hai so pehel se compute kerna hota


pc ki next movement in diff cases:

- Normal instruction: pc = nextPc (record ke baad wala).
- call: pc = foo ki position (jump).
- func_end: pc = returnLine (wapas caller mein, call ke baad wali instruction par).

each instruction:


func ke liye our frame is already made isliye yahan naya frame nahi banta. Sirf check hota hai ke header ka naam current frame ke naam se
match karta hai. Match na ho to "Unexpected function header" error


set (e.g  set k 10)  par pehle getValue se value 10 milti hai. Phir findVariable k dekhta hai ke k pehle se hai ya nahi, agar ho to value update hojati verna locals mein naya k banjata.


add/ sub/ mul/ div:  (e.g add b 5) par b ko dhoond kar uski value se result nikale ga


call: `call foo k` call par us record ke offset me call hone wala pc hota he hai jp target pc hai


func_end: current frame pop hota hai and stack khali ho gaya to main khatam verna parameters ki final values asli variables ki values ban jati hain ( jese foo mein b=15, main mein k=15 ki value hoti hai) or pc = returnLine.


## Changes after review:

Change 1: Extra stacks (sync aur limit issues wale) hatakar ab boundNames array aur frame.argc use kia hai.

Change 2: Literal arguments (jaise 5) par return hone wala "missing variable" error fix kar diya hai, ab variable na milne par copy back skip ho jata hai.

Change 3: Debugging behtar karne ke liye temporary printTimeline() function lagaya hai jo har snapshot par exact variable aur argument values print karta hai.
a dikhta. printTimeline
mein step 5 par foo{b=15} aur main{k=10} saaf dikhta hai, aur galat value foran pakri
jati.


# testing 0x2

<img width="940" height="818" alt="image" src="https://github.com/user-attachments/assets/fb73fdfb-38f4-4f9c-b9c9-b7c3eeb30c0c" />


# on 0x3

ab 0x3 mein timeline ke snapshots ko session.tdbg binary file mein save kerna hai.

0x2 mein snapshots memory mein store horahay thay, ab unko file mein write karenge taake baad mein access kar saken.

# writeInt32(), writeInt64(), writeString()

binary file mein integers write kerne ke liye hain orr strings ko directly write nahi ker sakte because string ke andar pointer waghera hota hai, so pehle length write ki then actual characters.

# writeVariable()

variable mein name and value hai, so writeString se name aur writeInt32 se value save ki.

# writeFrame()

frame mein function name, arguments, returnLine and local variables hotay hain.

so inko one by one write kiya. argv mein sirf argc jitne arguments and locals mein sirf localCount jitne variables not compelte array.

# writeSnapshot()

snapshot mein callStack hota hai, so pehle stackDepth write ki and phir jitne frames hain unko writeFrame se save kiya.

# writeHeader()

header mein TTDB magic, version, stepCount and indexOffset store hota hai.

indexOffset batata hai ke file mein index array kahan start hoti hai.

# writeTdbg()

pehle file open ki and placeholder header write kiya because indexOffset abhi pata nahi tha.

phir timeline ke nodes traverse kiye and har snapshot write karne se pehle ftell() se uski starting position index array mein store ki.

snapshots ke baad index array file mein write ki and actual indexOffset mil gaya.

last mein fseek se file ke start par wapas gaye and header update ker diya.

index ka purpose ye hai ke kisi bhi snapshot par directly jump kar saken without reading all previous snapshots.

# testing 0x3

pehle compilation error aya because writeInt32 and writeInt64 ki declarations writeHeader se pehle nahi thi, so function prototypes add kiye.

phir program successfully run hua and session.tdbg generate hogayi.

6 snapshots record huye and file size 276 bytes thi.

hexdump mein TTDB magic, version 1, stepCount 6 and indexOffset 228 mila.

6 index entries hain, each 8 bytes, so 228 + 48 = 276 which matches file size.

index offsets bhi check kiye, last snapshot 224 par start hota hai and index 228 par.

<img width="1288" height="886" alt="image" src="https://github.com/user-attachments/assets/135c89ab-d748-4549-a460-52ad6c083941" />

<img width="1986" height="300" alt="image" src="https://github.com/user-attachments/assets/0fb71bae-1587-4edc-9820-ec263758f12b" />

















