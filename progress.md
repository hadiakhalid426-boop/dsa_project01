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

ab 0x2 mein resolve.bin ki instructions actually execute kerni hain. 0x1 mein sirf function addresses resolve huye thay, ab variables update honge, function calls hongi and har instruction ke baad snapshot save hoga.

yahan hamara Stack and Timeline finally use horaha hai.

# tokenizeLine()

it divides oinstruction separat ewords me

stringstream use kiya for it

count total tokens batata hai. initially return count`miss hogaya tha  wo fix kiya. MAX_TOKENS also checked 

# buildSnapshot()

isme new Snapshot banate hain and snapshot_into() current callStack ke frames copy ker deta hai.

stackDepth batata hai kitne frames copy huye.

matlab har instruction ke baad us moment ki state save hojati hai. live stack baad mein change ho bhi jaye to old snapshot same rahega.

# findVariable()

variables do jagah ho sakte hain, argv mein parameters ya locals mein local variables.

so findVariable dono arrays mein search karta hai.

agar variable mil jaye to pointer return hota hai warna nullptr.

pointer isliye because original variable ki value update kerni hai yk

# getValue()

operand hamesha number nahi hota.

like add x y mein y variable ho sakta hai.

so pehle findVariable se search kerte hain agar variable hai to uski value return.

warna stol() se number mein convert kerte hain

invalid operand ya int32_t range se bahar value ho to error.

# executeProgram() start

resolve.bin ko rb mode mein open kiya because binary file read kerni hai.

then Stack<Frame> callStack banaya and mainFrame push kiya.

mainFrame ka returnLine -1 hai because main kisi aur function se call nahi hua.

pc = mainOffset

pc program counter hai, jo batata hai next instruction kis byte offset per read kerni hai.

# instruction reading loop

while loop tab tak chalega jab tak callStack empty nahi hojata.

fseek se pc wali position per jate hain, readResolveRecord se instruction read kerte hain, and ftell se nextPc milta hai.

then tokenizeLine instruction ko tokens mein divide karta hai.

normally pc nextPc per move karta hai, but call and func_end mein jump hota hai.

# func instruction

func main ya func foo encounter hone per dobara frame push nahi kerte.

main ka frame already push hai and foo ka frame call ke waqt banega.

so bas verify kerte hain ke function name current frame se match karta hai.

# set instruction

set k 10 mein pehle value getValue se milti hai.

then findVariable check karta hai ke k already exist karta hai ya nahi.

agar hai to update, warna locals mein create and localCount increase.


# add, sub, mul, div

sabko same else if mein handle kiya because logic similar hai.

target variable find kerte hain, operand ki value lete hain and operation perform kerte hain.

division by zero and arithmetic overflow checks bhi hain.

result temporarily int64_t mein calculate kiya because int32_t se bara ho sakta hai.

## call instruction

like call foo k

resolver ne foo ka address already call record mein patch kiya tha.

so recordOffset se targetPc milta hai.

fseek se foo ka header read kerte hain, jahan se parameter names milte hain.

newFrame banate hain, like foo ka parameter b hoga and uski initial value main ke k se ayegi.

returnLine mein nextPc store hota hai taake foo finish hone ke baad main mein wapas aa sakein.

then frame push hota hai and pc targetPc per jump karta hai.

## bindingStack and argumentCountStack

yahan problem thi ke foo mein parameter b hai but caller mein original variable k hai.

return per k ko update kerne ke liye original name remember karna zaroori hai.

extra struct use nahi karna tha, so two stacks banaye:

Stack<string> bindingStack

Stack<int32_t> argumentCountStack

bindingStack original argument names rakhta hai and argumentCountStack har call ke arguments ka count.

## func_end instruction

func_end per current frame pop hota hai.

agar callStack empty hojaye to main finish and execution stop.

warna caller frame milta hai.

bindingStack and argumentCountStack ki help se callee ke updated parameter values caller ke variables mein copy back kerte hain.

like foo mein b 10 se 15 hua to return per main ka k bhi 15 hojayega.

then pc = finished.returnLine so execution caller mein continue hogi.

reverse loop use kiya because stack LIFO follow karta hai.


## snapshots record kerna

har instruction ke baad timeline ka record update kerdete hain complete stack state save hojati hai.

like set k 10 ke baad k 10, call foo ke baad main and foo dono frames, and foo return ke baad main ka updated k.

isi tarha debugger previous states show kar sakega.

# testing 0x2



<img width="1120" height="270" alt="image" src="https://github.com/user-attachments/assets/42aedb21-c4a3-4813-a507-77bca2c5fcba" />

<img width="882" height="322" alt="image" src="https://github.com/user-attachments/assets/f95b0d53-648f-4351-8b4c-3913d4e92157" />














