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












