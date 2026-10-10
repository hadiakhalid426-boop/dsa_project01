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










