//iam writing this to test git


// ======================= TIME-TRAVEL DEBUGGER - SERVER TEMPLATE =======================

// Pipeline this file implements, top to bottom:
//   0. Receive  -- stream the client's .trace bytes straight to source.bin on disk
//   1. Pass 0X0   -- validity check (FUNC/FUNC_END matching)
//   2. Pass 0X1   -- resolve(): copy EVERY source line into resolve.bin as [offset][size][string], then patch CALL targets.
//   3. Pass 0X2   -- execute resolve.bin: tokenize ONE line at a time, update the call stack, take a snapshot -> Timeline
//   4. Pass 0X3   -- serialize Timeline -> session.tdbg(header + snapshot records + dense index)


#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

#include <sstream>
using namespace std;

// ---- Constants ----
const int32_t MAX_VARS_PER_FRAME = 16;
const int32_t MAX_STACK_DEPTH = 64;
const int32_t MAX_FUNCS = 128;
const int32_t MAX_TOKENS = MAX_VARS_PER_FRAME + 2; // kW + func_name + upto 16 params/args
const int32_t MAX_PATCHES = MAX_FUNCS * 4;
const uint64_t MAX_SOURCE_BYTES = 15ULL * 1024 * 1024; // sanity cap on the declared file length
const int32_t IO_BUFFER_SIZE = 64 * 1024;                  // fixed buffer for streaming to/from disk
const int32_t SOCKET_TIMEOUT_SEC = 5;                      // TODO: apply as SO_RCVTIMEO so a deadclient can't hang the server forever

// ---- Custom data structures

// Stack: back the live Call Stack during execution
template <typename T>
class Stack
{
    struct Node
    {
        T data;
        Node *next;
    };
    Node *top;
    int32_t count;

public:
    // Implement these functions:
    Stack()
    { // initialize the stack
        count =0;
        top = nullptr;

    }

    ~Stack(){
        Node *current =top;
        while(current!= nullptr){

            Node *nextNode = current->next;
            delete current;
            current=nextNode;


        }
    }
    void push(const T &val)
    {

        // pushes the value on the stack if max limit is not reached yet.
        if(count>=MAX_STACK_DEPTH){
            throw runtime_error("stack overflow (max stack depth)");
            return;

        }
        Node *newNode = new Node;

        newNode->data = val;
        newNode->next = top;

        top = newNode;

        count++;


    }

    T pop()
    {
        // pop the top value on the stack
        if(isEmpty()){
            return T();

        }

        T value =top->data;
        
        Node *temp = top;
        top= top->next;
        count--;
        //have to delete left out node verna reh jayega
        delete temp;
         
        return value;


    }
    T &peek()
    {
        if( isEmpty()){
            throw runtime_error("peek on an empty stack");

        }
        // returns the top value on the stack
        return top->data;


    }
    bool isEmpty()
    {
        return top == nullptr;
    }
    int32_t depth()
    {
        return count;
    }
    int32_t snapshot_into(T out[], int32_t maxLen)
    {
        // copies every frame, top to bottom in the array given as a parameter
        // this is what buildSnapshot() call, returns count written

        //my understanding while implementing this func:

        //so basically hamare pas frame aik current 
        // function ki state hoti hai, is tarha ham pure code ki
        //state agar aik stack me frame by frame kerke store kerleyin
        //to we aik scnapshot hoga ig?
        // A Snapshot is a copy of the entire call stack at one
        // particular moment during execution.
        //and then like kafi sare snapshots se timeline banti hai

        //meaning copied tells stack ke nodes/frames?

        Node *current=top;
        int32_t copied=0;

        while(current!= nullptr && copied<maxLen){
            out[copied] =current->data;
            copied++;

            current= current->next;

        }
        return copied;



    }
};


// Timeline : doubly linked list of Snapshots
struct Snapshot; // fwd declaration;
struct TimelineNode
{
    Snapshot *data;
    TimelineNode *next;
    TimelineNode *prev;
};
class Timeline
{
    TimelineNode *head, *tail;
    int32_t stepCount;

public:
    // Implement these functions
    Timeline()
    {

        head =nullptr;
        tail=nullptr;

        stepCount =0;

    }
    void record(Snapshot *s)
    {
        // add record in the timeline
        //matlab add a snapshot already existing record mein
        //timeline ka node create kia

        TimelineNode *newNode = new TimelineNode;

        newNode->data= s;
        newNode->next=nullptr;

        //we are updating tail if else ke andar
        
        if (head == nullptr){
            head =newNode;
            tail= newNode;
            newNode->prev = nullptr;
        }
        else{

            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;

        }
        
        stepCount++;

    }
    TimelineNode *begin()
    {
        return head;


    }
    int32_t getStepCount()
    {
        return stepCount;

    }
};

// Core structs
struct Variable
{
    string name;
    int32_t value;
};
struct Frame
{
    string func_name;
    int32_t argc;
    Variable argv[MAX_VARS_PER_FRAME];
    int32_t returnLine;
    Variable locals[MAX_VARS_PER_FRAME];
    int32_t localCount;
};

struct Snapshot
{
    Frame callStack[MAX_STACK_DEPTH];
    int32_t stackDepth;
};

struct TTDBHeader
{
    char magic[4]; // "TTDB"
    int32_t version;
    int32_t stepCount;
    int64_t indexOffset;
};

void writeHeader(FILE *f, const TTDBHeader &h)
{
    fwrite(h.magic, 1, 4, f);
    fwrite(&h.version, sizeof(int32_t), 1, f);

    // placeholder for other two data members

}

// resolve.bin - bookkeeping
struct FuncEntry
{
    string funcName;
    int64_t byteOffsetInResolveBin; // where this function's FUNC header record sits
};
struct PendingPatch
{
    int64_t byteOffsetOfOffsetField; // where in resolve.bin to seek back and overwrite
    string targetFuncName;
};



// PASS 0x0: READING source.bin + VALIDITY CHECK
bool readSourceLine(ifstream &in, string &out)
{
    // reads the next nonblank line


    //ham normally getline use kerte hain and if statements
    //se blanks ko ignore kerte hai

    while(getline(in, out)){
        bool blank= true;

        for(size_t i=0; i<out.length(); i++){
            char c =out[i];

            if( c!= ' ' && c!= '\t' && c!= '\r'){

                blank= false;
                break;
            }

        }

        if(blank == false){

            return true;
        }
    }
    return false;

}
string firstWord(const string &line)
{
    // returns first word from the input string

    //including sstream for it so taking a word out of line is easy

    stringstream list(line);

    string word;
    list>> word;

    return word;

}

string secondWord(const string &line)
{
    // returns the second word
    stringstream list(line);

    string word1;
    string word2;
    list>> word1;
    list>>word2;


    return word2;
}
bool validateProgram(const char *sourcePath)
{
    // for each func defined there should be exactly one func_end and no nested funcs allowed - 

    ifstream in(sourcePath);

    if(! in.is_open()){
        cout<<"error : file not open"<<endl;
        return false;

    }

    string line;
    bool inside= false;
    //to check ifmwe have entered a func or not

    while(readSourceLine(in, line)){
        string keyword= firstWord(line);
        //technically just read line by line (skippign space ofc)

        if(keyword =="func"){
            if(inside == true){
                //meanign. pehle se he tru nai hona chaiye
                cout<< "error : nested func not allowed";
                return false;

            }
            inside=true;


        }
        else if( keyword== "func_end"){
            if(inside == false){
                //func ke andar hona chaiye

                cout<<"error: func_end cannot exist without func";
                return false;


            }
            inside=false;

        }
    }
    //reachign file end without its closing:

    if(inside){
        cout<< "error: missing func end";
        return false;

    }

    return true;
    //this resembels validating bracket probleam but here nested
    //he allow nai hain, verna stack use kerte
    
}

// PASS 0x1: RESOLVE() -> resolve.bin
int64_t writeResolveRecord(FILE *f, int64_t offsetField, const string &text)
{
    // writes one [offset(8B)][size(4B)][string] record at the current file position
    // returns this record's own starting byte position

    //to return we have to store before kaha par file write keregi yk
    //usinf ftell
    int64_t start= ftell(f);

    //first of all writing down offset jiska size 8 bytes ka hota
    fwrite(&offsetField, sizeof(int64_t), 1, f);
    //now we will write size of string, which is not given so we
    //calculate size of string 
    // we could write this: int32_t size= text.size();
    //but yaha size field ki wajha se ham intentioanlly convert kerenge:
    int32_t size =static_cast<int32_t>(text.size());
    fwrite(&size, sizeof(int32_t), 1, f); //4 bytes yaha
    //now write complete string
    fwrite(text.data(), 1, size, f); //each item is of 1 byte

    return start;





}
int64_t readResolveRecord(FILE *f, string &outText)
{
    // reads one record at the current position and advances past it, returns the offset field - the raw line text comes back untouched in outText.
    int64_t offset;
    int32_t size;
    //it is possible that we try reading file when its at its end


    if(fread(&offset, sizeof(int64_t), 1, f) !=1){
        return -1;

    }
    if(fread(&size, sizeof(int32_t), 1, f)!=1){
        return -1;

    }
    //invalid size bhe ho sakta hai
    if(size<0 || size> MAX_SOURCE_BYTES){
        return -1;

    }


    //string ko size ka kerna jis size ka text likha hua so:
    outText.resize(size);

    //fread returns number of complete items successfully read
    //isliye =1 wali condition hai

    if(size > 0){
        if(fread(&outText[0], 1, size, f)!= static_cast<size_t>(size)){
            return -1;

        }
    }

    return offset;

}
int64_t resolveProgram(const char *sourcePath, const char *resolveBinPath)
{
    FuncEntry funcArray[MAX_FUNCS];
    int32_t funcCount = 0;
    PendingPatch patches[MAX_PATCHES];
    int32_t patchCount = 0;
    // Every source line becomes one record holding the raw line, as-is.
    // resolve() only PEEKS at the leading word(s) -- enough to spot FUNC
    // (remember its position) and CALL (remember which function it needs
    // and where its offset field sits).
    // Once the whole file is written, every CALL's offset field is patched
    // with its target's position. Patching happens after the full write
    // Returns the byte offset of main's FUNC header record.
    // if there is no main return the error 

    //first lets just open the files:
    ifstream in(sourcePath);

    if(!in.is_open()){

        cout<<"error: source file not open file"<<endl;
        return -1;
    }

    FILE *out= fopen(resolveBinPath, "wb+");

    if(out== nullptr){
        cout<<"error: cannot create resolve bin"<<endl;
        return -1;

    }

    string line;
    int64_t currentOffset = 0;

    while(readSourceLine(in, line)){
        string keyword=firstWord(line);

        int64_t nextOffset = currentOffset + sizeof(int64_t) + sizeof(int32_t) + static_cast<int64_t>(line.size());

        int64_t storedOffset= currentOffset;

        if (keyword == "call") {
            storedOffset = 0;
        }

        //records should have their next record offset, ye 0 placeholder hai
        // while call records will contain their 
        // function target

        int64_t recordStart= writeResolveRecord(out, storedOffset, line);


        if(keyword== "func"){

            string functionName =secondWord(line);


            if (functionName.empty()){
                cout << "Error: function name missing.\n";
                fclose(out);
                return -1;
            }

            for (int32_t j = 0; j < funcCount; j++)
            {
                if (funcArray[j].funcName == functionName)
                {
                    cout << "Error: duplicate function "<< functionName << endl;
                    fclose(out);
                    return -1;
                }
            }

            if(funcCount>=MAX_FUNCS){

                cout<<"ERROR: LIMIT OF FUNCS"<<endl;
                fclose(out);
                return -1;


            }

            funcArray[funcCount].funcName= functionName;
            funcArray[funcCount].byteOffsetInResolveBin = recordStart;
            funcCount++;

        }
        else if(keyword == "call"){

            string targetName= secondWord(line);

            if (targetName.empty()){
                cout << "Error: CALL target missing.\n";
                fclose(out);
                return -1;
            }

            if(patchCount>=MAX_PATCHES){

                cout<<"error: patch limit"<<endl;
                fclose(out);
                return -1;

            }
            patches[patchCount].targetFuncName= targetName;
            patches[patchCount].byteOffsetOfOffsetField = recordStart;
            patchCount++;

            //meaning this byte needs foo this needs faa

        }

        //now agla record: start hoga
        //8 byte ka offset+ 4 byte size+ string bytes 
        currentOffset=nextOffset;




    }

    if (in.bad()){

        cout << "Error: failed to read source file.\n";
        fclose(out);
        return -1;
    }
    //now hamne pending call instructions ko patch kerna hai or
    //main ka offset return kerna hai


    for(int32_t i=0; i<patchCount; i++){
        int64_t targetOffset= -1;

        for(int32_t j=0; j<funcCount; j++){

            if(patches[i].targetFuncName == funcArray[j].funcName){

                targetOffset=funcArray[j].byteOffsetInResolveBin;
                break;
            }
        }

        if(targetOffset ==-1){
            cout<<" error: no function exist(undefined )"<< patches[i].targetFuncName<<endl;

            fclose(out);
            return -1;
                

        }
        //now change call recor doffset filed
        if (fseek(out, static_cast<long>(patches[i].byteOffsetOfOffsetField),SEEK_SET) != 0){

            //seek set tells where to start: beginning 
        
            cout << "Error: failed to seek to CALL record.\n";
            fclose(out);
            return -1;
        }

        // Replace the placeholder with the target function's offset.
        if (fwrite(&targetOffset, sizeof(int64_t), 1, out) != 1){
            //here 1 tells ke one item wrote
            cout << "Error: failed to patch CALL record.\n";
            fclose(out);
            return -1;
        }
    }


    int64_t mainOffset = -1;
    //main ka offset return kerna hai

    for (int32_t i = 0; i < funcCount; i++){

        if (funcArray[i].funcName == "main"){
            mainOffset = funcArray[i].byteOffsetInResolveBin;
            break;
        }
    }   

    if (mainOffset == -1){
        cout << "Error: main function not found.\n";
        fclose(out);
        return -1;
    }   
    fclose(out);
    return mainOffset;

}

// PASS 0x2: EXECUTION (tokenization happens here)
enum TokenType
{
    KEYWORD,
    IDENTIFIER,
    PARAM
};
struct Token
{
    TokenType type;
    string text;
};
int32_t tokenizeLine(const string &line, Token tokens[], int32_t maxTokens)
{
    stringstream ss(line);
    string word;
    int32_t count=0;

    while(ss>> word){
        if(count>= maxTokens){
        throw runtime_error("too many tokens");

        }
        tokens[count].text = word;

        if(count==0){
            tokens[count].type =KEYWORD;

        }
        else if(count ==1){
            tokens[count].type= IDENTIFIER;

        }
        else{
            tokens[count].type= PARAM;
        }

        count++;
    
    }

    return count;
    //taake executor ko pata ho kitne tokens valid hain

    // first word is always a instruction keyword
    // instruction set = [func, func_end, call, set, add, sub, mul and div]
    // next word is identifier like name of a function, variable name
    // after identifier all are the params/arg, space separated
}
Snapshot *buildSnapshot(Stack<Frame> &callStack)
{
    // build the snapshot based on the callStack given
    Snapshot* snap= new Snapshot;

    snap->stackDepth= callStack.snapshot_into(snap->callStack, MAX_STACK_DEPTH);

    return snap;
}

//varibales can be at twp places, ya to parameter ho sakte ya local func ke andar bane
//hosakte isliey we are making this helper
Variable* findVariable(Frame &frame, const string &name)
{
    for (int32_t i = 0; i < frame.argc; i++)
    {
        if (frame.argv[i].name == name)
        {
            return &frame.argv[i];
        }
    }

    for (int32_t i = 0; i < frame.localCount; i++)
    {
        if (frame.locals[i].name == name)
        {
            return &frame.locals[i];
        }
    }

    return nullptr;
}//ye dono jagha search kwer leta

//ab e.g add x y mein y ko number treat kerna hai ya variablename isliye

int32_t getValue(Frame &frame, const string &operand)
{
    Variable *v = findVariable(frame, operand);

    if (v != nullptr)
    {
        return v->value;
    }

    size_t pos = 0;

    try
    {
        long value = stol(operand, &pos);

        if (pos != operand.size() ||
            value < INT32_MIN ||
            value > INT32_MAX)
        {
            throw runtime_error("Invalid operand: " + operand);
        }

        return static_cast<int32_t>(value);
    }
    catch (const invalid_argument &)
    {
        throw runtime_error("Undefined variable: " + operand);
    }
    catch (const out_of_range &)
    {
        throw runtime_error("Number out of range: " + operand);
    }
}

void executeProgram(const char *resolveBinPath, int64_t mainOffset, Timeline &timeline)
{

    FILE *f = fopen(resolveBinPath, "rb");

    if (f == nullptr){
        throw runtime_error("Cannot open resolve.bin");
    }
    // initialize the call stack
    Stack<Frame> callStack;
    // make the main frame
    Frame mainFrame{};
    mainFrame.func_name = "main";
    mainFrame.argc = 0;
    mainFrame.localCount = 0;
    mainFrame.returnLine = -1;
    // push main frame on the call stack
    callStack.push(mainFrame);

    int64_t pc=mainOffset;

    Stack<string> bindingStack;
    Stack<int32_t> argumentCountStack;

    while(!callStack.isEmpty()){

        //move kerenge instruction per jo execute kerni:
        if (fseek(f, static_cast<long>(pc), SEEK_SET) != 0){
            fclose(f);
            throw runtime_error("Failed to seek instruction");
        }

        string line;
        int64_t recordOffset = readResolveRecord(f, line);

        if (recordOffset == -1){
            fclose(f);
            throw runtime_error("Failed to read instruction");
        }

        //ab ftell point kerega agle recor dko

        long nextPosition =ftell(f);
        if(nextPosition <0){
            fclose(f);
            throw runtime_error("failed to get next position");
        }
        
        int64_t nextPc = static_cast<int64_t>(nextPosition);

        Token tokens[MAX_TOKENS];
        int32_t tokenCount = tokenizeLine(line, tokens, MAX_TOKENS);

        if (tokenCount == 0){       
            pc = nextPc;
            continue;
        }

        string keyword = tokens[0].text;
        cout << "Executing: " << line << endl;

        Frame &current = callStack.peek();

        if (keyword == "func"){

        // The frame was already created by main initialization
        // or by a call instruction.
            if (tokenCount < 2 || tokens[1].text != current.func_name){
                throw runtime_error("Unexpected function header");
            }
        }

        else if (keyword == "set"){

            if (tokenCount != 3){
                throw runtime_error("Invalid set instruction");
            }

            string name = tokens[1].text;
            int32_t value = getValue(current, tokens[2].text);

            Variable *v = findVariable(current, name);

            if (v != nullptr){

                v->value = value;
            }
            else{
                if (current.localCount >= MAX_VARS_PER_FRAME){

                    throw runtime_error("Too many local variables");
                }

                current.locals[current.localCount].name = name;
                current.locals[current.localCount].value = value;
                current.localCount++;
            }
        }
        else if (keyword == "add" || keyword == "sub" || keyword == "mul" || keyword == "div"){
            
            if (tokenCount != 3){

                throw runtime_error("Invalid arithmetic instruction");
            }

            Variable *target = findVariable(current, tokens[1].text);

            if (target == nullptr){

                throw runtime_error("Undefined target variable");
            }

            int32_t operand = getValue(current, tokens[2].text);

            int64_t result = target->value;

            if (keyword == "add")
            {
                result += operand;
            }
            else if (keyword == "sub")
            {
                result -= operand;
            }
            else if (keyword == "mul")
            {
                result *= operand;
            }
            else if (keyword == "div")
            {       
                if (operand == 0)
                {
                    throw runtime_error("Division by zero");
                }

                result /= operand;
            }

            if (result < INT32_MIN || result > INT32_MAX){

                throw runtime_error("Arithmetic overflow");
            }

            target->value = static_cast<int32_t>(result);
        }       

        else if (keyword == "call"){

            if (tokenCount < 2)
            {
                throw runtime_error("Invalid call instruction");
            }

            int32_t argumentCount = tokenCount - 2;

            if (argumentCount > MAX_VARS_PER_FRAME)
            {
                throw runtime_error("Too many arguments");
            }       

            if (callStack.depth() >= MAX_STACK_DEPTH){
                throw runtime_error("Stack overflow");
            }

            // recordOffset contains the patched function address.
            int64_t targetPc = recordOffset;

            if (targetPc < 0){

                throw runtime_error("Invalid call target");
            }

            // Read the target function's header.
            if (fseek(f, static_cast<long>(targetPc), SEEK_SET) != 0){

                throw runtime_error("Cannot seek to called function");
            }

            string headerLine;
            if (readResolveRecord(f, headerLine) ==-1){

                throw runtime_error("Failed to read function header");

            }
            Token headerTokens[MAX_TOKENS];
            int32_t headerCount =
                tokenizeLine(headerLine, headerTokens, MAX_TOKENS);

            if (headerCount < 2 || headerTokens[0].text != "func" || headerTokens[1].text != tokens[1].text){
                throw runtime_error("Invalid function target");
            }

            int32_t parameterCount = headerCount - 2;

            if (parameterCount != argumentCount){

                throw runtime_error("Argument count mismatch");
            }

            if (nextPc > INT32_MAX){

                throw runtime_error("Return address out of range");
            }

            Frame newFrame{};
            newFrame.func_name = headerTokens[1].text;
            newFrame.argc = argumentCount;
            newFrame.localCount = 0;
            newFrame.returnLine = static_cast<int32_t>(nextPc);

            for (int32_t i = 0; i < argumentCount; i++){

                newFrame.argv[i].name = headerTokens[i + 2].text;
                newFrame.argv[i].value = getValue(current, tokens[i + 2].text);
            }

            for (int32_t i = 0; i < argumentCount; i++){

                bindingStack.push(tokens[i + 2].text);
            }

            argumentCountStack.push(argumentCount);

            callStack.push(newFrame);

            // Jump to the called function's header.
            pc = targetPc;
        }   
        else if (keyword == "func_end"){

            Frame finished = callStack.pop();

            if (callStack.isEmpty()){
                timeline.record(buildSnapshot(callStack));
                break;
            }

            Frame &caller = callStack.peek();

            int32_t count = argumentCountStack.pop();

            for (int32_t i = count - 1; i >= 0; i--){
                string callerName = bindingStack.pop();

                Variable *original = findVariable(caller, callerName);

                if (original == nullptr){
                    throw runtime_error("Missing caller variable");
                }

                original->value = finished.argv[i].value;
            }

            pc = finished.returnLine;
        }
        else{

            throw runtime_error("Unknown instruction: " + keyword);
        }

        if (keyword != "call" && keyword != "func_end"){

            pc = nextPc;

        }

        // save kernge current call stack state
        timeline.record(buildSnapshot(callStack));

    }


    fclose(f);

    // implementation:
    // execute line by line, and according to the keyword perform action
}

// PASS 0x3: SERIALIZE TIMELINE
void writeTdbg(Timeline &timeline, const char *tdbgPath)
{
    // placeholder for header
    // index array of the size of stepcount from the timeline
    // placing each snapshot in the file while maintaining the index(starting point of each nth snapshot)
    // after timeline add the index array i the file
    // update the header
}

// main section
int32_t main()
{

    if (!validateProgram("source.bin"))
    {
        // send an error response instead of a .tdbg file
        return 1;
    }

    int64_t mainOffset = resolveProgram("source.bin", "resolve.bin");

    cout << "mainOffset = " << mainOffset << endl;
    if(mainOffset <0){
        cout<<"resolver failed"<<endl;
        return 1;

    }


    Timeline timeline;
    try{

        executeProgram("resolve.bin", mainOffset, timeline);
    }
    catch( const exception &e){
        cout<<"error: "<<e.what()<<endl;
        return 1;
    }

    // writeTdbg(timeline, "session.tdbg");

    return 0;
}



// int32_t main()
// {
//     cout << "Program started!" << endl;

//     bool valid = validateProgram("source.bin");

//     cout << "Validation result = " << valid << endl;

//     if (!valid)
//     {
//         cout << "Validation failed!" << endl;
//         return 1;
//     }

//     int64_t mainOffset = resolveProgram("source.bin", "resolve.bin");

//     cout << "mainOffset = " << mainOffset << endl;

//     if (mainOffset < 0)
//     {
//         cout << "Resolver failed!" << endl;
//         return 1;
//     }

//     return 0;
//}