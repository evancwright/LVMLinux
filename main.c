#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 80
#define SCORE_INDENT 48
#define CONSOLE 3 
#define IXREGISTER 4 
 
unsigned char A=0;
unsigned char B=0;
unsigned char D=0;
unsigned char E=0;
unsigned short IX=0;
unsigned short IY=0;

unsigned char ZeroFlag=1;
unsigned char GTFlag=0;
unsigned char LTFlag=0;
unsigned char NegFlag=0;

unsigned char ScreenWidth  = 40;

unsigned char ScreenHeight = 40;

unsigned short ObjectTable=0;
unsigned short StringTable=0;

unsigned short PC=0;
unsigned short SP=0;
unsigned char IR=0;

unsigned char * memory;

unsigned char Immediate8 = 0;
unsigned short Immediate16 = 0;
unsigned int RamSize=0;
unsigned type = 0;
unsigned subType = 0;
unsigned mode = 0;
unsigned char hPos = 0;
unsigned short bufferAddr=0;
unsigned short bufferStart = 0;
unsigned char  outputChannel = CONSOLE;
const int OBJECT_ENTRY_SIZE = 21;
const int PROPERTY_BYTE_1 = 19;
const int PROPERTY_BYTE_2 = 20;
unsigned short  masks[] = {0, 1, 2, 4, 8, 16, 32, 64, 128,
		1, 2, 4, 8, 16, 32, 64, 128 };

int min(int,int);
void Init();
void Fetch();
void Decode();
unsigned char FetchByte();
void Execute();
void Load8();
void Compare8();
void Add8();
void AddIX();
void AddIY();
unsigned short GetAdd16Data();
void Inc();
void Dec();
unsigned char GetSource8();
unsigned char GetTarget8();
void TakeBranch();
void TakeJump();
void Return();
void SetFlags(unsigned char tgt, unsigned char data);
void SetReg8(unsigned char data);
void Status();
void Sub8();
void And8();
void Mul();
void Branch();
void Jump();
void Or8();
void StoreRIXNN();
void PushPC();
void Pop();
unsigned char Pop8();
void PopDE();
void PopReg8();
void PopPC();
void PopAll();
void Push();
void Push8();
void Push8ToStack(unsigned char);
void PushDE();
void PushIX();
void PushIY();
void PushAll();
void CallOrReturn();
void DoCall();
void DoCallIX();
unsigned short MoveNextWord(unsigned short i);
unsigned short PrintWord(unsigned short i);
void LoadIX();
void LoadIY();
void LoadSP();
void SpecialOps1();
void SpecialOps2();
void SpecialOps3();
void PrintStrN16();
void PrintStrN16Cr();
void CompareIndexed();
void StoreRIY();
void StoreIXIY();
void TestOrMul();
void GetObjProp();
void GetObjAttr();
void GetObjAttr16();
void SetObjAttr();
void SetObjProp();
void SetObjAttr16();
void GetParent();
unsigned short GetLoad16Data();
void CharOut();
void ChOut(unsigned char ch);
void Newline();
void ReadLine();
void Quit();
void RMod();
void Save();
void Restore();
void PrintStrPtr();
void PrintStrPtrCr();
int WordLenIX(unsigned short i);
void SetEnv();
void Streq();
void CLS();
void AnyKey();
void Streq();
void LoadIndexed();
void RageDump();
void GetDbgCmd();

unsigned short GetHex(char *);

unsigned char ucase(unsigned char); 

char buffer[80];
char cmdbuf[80];
char topline[80];
unsigned char debug=1;
unsigned short breakPoints[10];
int numBp=0;
int step=0;
int bpOn=0;
		int main(int argc, char **argv)
		{ 
			int i=0;
			FILE *fp = 0;
			
			
			if (argc !=  2)
			{
				printf("Usage: lvm file\n");
			}
			
			fp = fopen(argv[1],"rb");
			if (fp!=0)	
			{
			
				fseek(fp, 0, SEEK_END);
				RamSize = ftell(fp);
				rewind(fp);
			//	fseek(fp, 0, SEEK_SET);
				memory = malloc(RamSize);
					
				if (memory == 0)
				{
					printf("Unable allocate ram");
					exit(0);
				}
					
					RamSize = fread(memory,1,RamSize,fp);
				fclose(fp);
				Init();
				

				 

				while (1)
				{
			//		if (PC == 0x0016)
				//	{
					//	bpOn = 1;
				//	}
					
			//		if (bpOn==1)
			//		{
			//			printf("PC=%#08x",PC);
			//		}
			
					if (step == 1)
					{
						GetDbgCmd();
						
						Fetch();
						Execute();
						RageDump(); 
 					}
					else
					{
						for (i=0; i < numBp; i++)
						{
							if (PC == breakPoints[i])
							{
								printf("breakpoint at %#04x\n", breakPoints[i]);
								step = 1; 
								RageDump();
								break;
							}
						}

						
						Fetch();
						Execute();
						
						
					}
				}
				
				free(memory);
			}
			else
			{
				printf("Unabled to open file.\r\n");
				exit(0);
			}
			
			return 0;
		}
	 
   
        void Init()
        { 
        } 
       
        void Fetch()
        {
            IR = memory[PC];
            PC++;

            Decode();
        }

        void Decode()
        {
            type = ((IR & 0xE0) >> 5) ; //0x11100000
            subType = (IR & 0x18) >> 3; //0x00011000
            mode = IR & 0x07; //x00000111

            if (type == 0 || type == 1 || type == 2)
            {//load
                if (mode == 4)
                {//one unsigned char immediate
                    Immediate8 = FetchByte();
                }
                else if (mode == 6)
                {//two unsigned char immediate
                    Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                }
            }
            else if (type == 3)
            {
                if (subType == 0)
                {//branch
                    Immediate8 = FetchByte();
                }
                else if (subType == 1)
                {//jump
                    Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                }
                else if (subType == 2)
                {//call /ret 
                    if (mode == 0 || mode == 1 || mode == 6)
                    {
                        Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                    }
                }
                else if (subType == 3)
                {
                    //these are tests - no immediate data
                }
            }
            else if (type == 4)
            {//inc dec add
                //inc / dec (nn)
                if (subType == 0 || subType == 1)
                {
                    if (mode == 4)
                    { //dec / inc (nn)
                        Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                    }
                }
                else if (subType == 2 || subType == 3)
                {//add16
                    if (mode == 6)
                        Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte()); 
                }
            }
            else if (type == 5)
            {//store 
                if (subType == 2)
                {
                    if (mode <= 3)
                    {// st r,(nn)
                        Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                    }
                }
                else if (subType == 3)
                {
                    if (mode == 6)
                    {//load sp,nn
                        Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                    }
                }
            }
            else if (type == 6)
            {
                if (subType == 0)
                {//store ix,iy (nn)
                    if (mode == 5 || mode == 7)
                    {
                        Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                    }
                }
                else if (subType == 1 || subType == 2)
                {// lda 3,(iy) or (iy)
                    Immediate8 = FetchByte();
                }
                else if(subType == 3)
                {//LD IX/SP,??
                    if (mode == 0 || mode == 1 || mode == 6) //ld ix,nn
                    {
                        Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                    }
                }
            }
            else if (type == 7)
            {
                if (subType == 0 )
                {//ld iy and sp
                    if (mode == 0 || mode == 1 || mode == 6)
                    {
                        Immediate16 = (unsigned short)(FetchByte() * 256 + FetchByte());
                    }
                }
            }
        }

        unsigned char FetchByte()
        {
            unsigned char b = memory[PC];
            PC++;
            return b;
        }

        void Execute()
        {
			 if (type == 0) Load8();
			else if (type == 1)
			{
				Compare8();
			}
			else if (type == 2)
			{
				if (subType == 0) Add8();
				else if (subType == 1) Sub8();
				else if (subType == 2) And8();
				else if (subType == 3) Or8();
			}
			else if (type == 3)
			{
				if (subType == 0) Branch();
				else if (subType == 1) Jump();
				else if (subType == 2) CallOrReturn();
				else if (subType == 3) TestOrMul();
			}
			else if (type == 4)
			{
				if (subType == 0) Inc();
				else if (subType == 1) Dec();
				else if (subType == 2) AddIX();
				else if (subType == 3) AddIY();
			}
			else if (type == 5) //101
			{
				if (subType == 0) Push();
				else if (subType == 1) Pop();
				else if (subType == 2) StoreRIXNN();
				else if (subType == 3) StoreRIY();
			}
			else if (type == 6)
			{
				if (subType == 0) StoreIXIY();
				else if (subType == 1) LoadIndexed();
				else if (subType == 2) CompareIndexed();
				else if (subType == 3) LoadIX();
			}
			else if (type ==7)
			{
				if (subType == 0) LoadIY();
				else if (subType == 1) SpecialOps1(); ///????
				else if (subType == 2) SpecialOps2();
				else if (subType == 3) SpecialOps3();
			}
		}

        void Load8()
        {
            unsigned char data = GetSource8();
            SetReg8(data);
        }

        void Compare8()
        {
            unsigned char data = GetSource8();
            unsigned char tgt = GetTarget8();

            SetFlags( tgt, data);
        }

        void SetFlags(unsigned char tgt, unsigned char data)
        {
            ZeroFlag = 0;
            if (tgt == data) ZeroFlag = 1;

            if (tgt > data) GTFlag = 1;
            else GTFlag = 0;

            if (tgt < data) LTFlag = 1;
            else LTFlag = 0;
        }

        void SetReg8(unsigned char data)
        {
            if (subType == 0) A = data;
            else if (subType == 1) B = data;
            else if (subType == 2) D = data;
            else if (subType == 3) E = data;
            
            if (data == 0)
                ZeroFlag = 1;
            else
                ZeroFlag = 0;

            if (data < 128)
                NegFlag = 0;
            else
                NegFlag = 1;

        }

        void Add8()
        {
            unsigned char data = GetSource8(); 
            A = (unsigned char)(A + data);
            if (A == 0) ZeroFlag = 1;
            else ZeroFlag = 0;
        }

        void Sub8()
        {
            unsigned char data = GetSource8();
            A = (unsigned char)(A - data);
            if (A == 0) ZeroFlag = 1;
            else ZeroFlag = 0;
        }

        void And8()
        {
            unsigned char data = GetSource8();
            A = (unsigned char)(A & data);

            if (A == 0) ZeroFlag = 1;
            else ZeroFlag = 0;
        }

        void Or8()
        {
            unsigned char data = GetSource8();
            A = (unsigned char)(A | data);

            if (A == 0) ZeroFlag = 1;
            else ZeroFlag = 0;
        }

        /// <summary>
        /// Store R in IX or NN
        /// </summary>
        void StoreRIXNN()
        {
           int reg = mode & 3;
            unsigned char data = 0;
            if (reg == 0)
                data = A;
            else if (reg == 1)
                data = B;
            else if (reg == 2)
                data = D;
            else if (reg == 3)
                data = E;

            if (mode < 4)
            {
                memory[Immediate16] = data;
            }
            else  
            {
                memory[IX] = data;
            }
            
        }

        /// <summary>
        /// Store R in IY
        /// </summary>
        void StoreRIY()
        {
			unsigned char data = 0;
            mode = mode & 3;
            
            if (mode == 0)
                data = A;
            else if (mode == 1)
                data = B;
            else if (mode == 2)
                data = D;
            else if (mode == 3)
                data = E;

            memory[IY] = data;
        }

        void TestOrMul()
        {
            /*
             * TEST	pushzf	011	pushzf	11		000		120	0	1
TEST	pushlt	011	pushlt	11		001		121	0	1
TEST	pushlte	011	pushlte	11		010		122	0	1
TEST	pushgt	011	pushgt	11		011		123	0	1
TEST	pushgte	011	pushgte	11		100		124	0	1
RET	???	011		11		101		125	0	1
RET	???	011		11		110		126	0	1
MUL	MUL	011	MUL	11		111		127	0	1*/
            
            if (mode == 0)
            {//push zf
                memory[SP] = ZeroFlag;
                SP--;
                memory[SP] = ZeroFlag;
                SP--;
            }
            else if (mode == 1)
            {//pushlt
                memory[SP] = LTFlag;
                SP--;
                memory[SP] = LTFlag;
                SP--;
            }
            else if (mode == 2)
            {//pushlte
                memory[SP] = (unsigned char)(LTFlag | ZeroFlag); ;
                SP--;
                memory[SP] = (unsigned char)(LTFlag | ZeroFlag); ;
                SP--;
            }
            else if (mode == 3)
            {//pushgt
                memory[SP] = GTFlag;
                SP--;
                memory[SP] = GTFlag;
                SP--;
            }
            else if (mode == 04)
            {//push gte
                memory[SP] = (unsigned char)(GTFlag | ZeroFlag);
                SP--;
                memory[SP] = (unsigned char)(GTFlag | ZeroFlag);
                SP--;
            }
            if (mode == 5)
            {//push zf
                unsigned char temp = 0;
                if (ZeroFlag == 0)
                    temp = 1;

                memory[SP] = temp;
                SP--;
                memory[SP] = temp;
                SP--;
            }
            else if (mode == 7)
            {
                Mul();
            }
        }

        void Mul()
        {
            unsigned short prod = (unsigned short)(A*B);
            A = (unsigned char)(prod / 256);
            B = (unsigned char)(prod % 256);
        }

        void Branch()
        {
            /*
             * 000  = z
001 = nz
010 = lt
011 = gt
100 = lte
101 =gte
110
111
*/
            if (mode == 0 && ZeroFlag == 1) TakeBranch();
            else if (mode == 1 && ZeroFlag == 0) TakeBranch();
            else if (mode == 2 && LTFlag == 1) TakeBranch();
            else if (mode == 3 && GTFlag == 1) TakeBranch();
            else if (mode == 4 && (LTFlag == 1 || ZeroFlag == 1 )) TakeBranch();
            else if (mode == 5 && (GTFlag == 1 || ZeroFlag == 1 )) TakeBranch(); //bad op code
            else if (mode == 7 ) TakeBranch();
        }


        void Jump()
        {
            /*
            * 000  = z
001 = nz
010 = lt
011 = gt
100 = lte
101 =gte
110
111
*/
            if (mode == 0 && ZeroFlag == 0) TakeJump();
            else if (mode == 1 && ZeroFlag == 1) TakeJump();
            else if (mode == 1 && LTFlag == 1) TakeJump();
            else if (mode == 1 && GTFlag == 1) TakeJump();
            else if (mode == 1 && (LTFlag == 1 || ZeroFlag == 1)) TakeJump();
            else if (mode == 1 && (GTFlag == 1 || ZeroFlag == 1)) TakeJump();
            else if (mode == 7)
            {
                TakeJump();
            }
        }

        void CallOrReturn()
        {
//			printf("Call or return, mode=%d\n",mode);
            if (mode == 0 && ZeroFlag==1) DoCall();
            else if (mode == 1 && ZeroFlag==0) DoCall();
            else if (mode == 5) DoCallIX();
			else if (mode == 2 || mode == 3)
			{
				printf("Invalid mode %d. PC=%#04x\n", mode, PC);
				exit(0);	
			}
            else if (mode == 6) DoCall();
            else if (mode == 7) Return();
        }

        void DoCall()
        {
//			printf("calling  %#08x\n", Immediate16);
            PushPC();
            PC = Immediate16;
        }

        /// <summary>
        /// Call the subroutines stored at the address in IX
        /// </summary>
        void DoCallIX()
        {
			unsigned char PCHi=0;
			unsigned char PCLo=0;
            PushPC();
            PCHi = memory[IX];
            PCLo = memory[IX + 1];
            PC = (unsigned short)(PCHi * 256 + PCLo);
        }

     

        void PushPC()
        {
            unsigned char PCLo = (unsigned char)(PC % 256);
            unsigned char PCHi = (unsigned char)(PC / 256);
            memory[SP] = PCLo;
            SP--;
            memory[SP] = PCHi;
            SP--;
        }

        void PopPC()
        {
			unsigned char hi=0;
			unsigned char lo=0;
            SP++;
            hi = memory[SP];
            SP++;
            lo  = memory[SP]; 
            PC = (unsigned short)(hi * 256 + lo);
        }

        void Return()
        {
//			printf("returning\n");
            PopPC();
        }

        void PopIX()
        {
			unsigned char lo,hi;
            SP++;
            hi = memory[SP];
            SP++;
            lo = memory[SP];
            
            IX = (unsigned short)(hi * 256 + lo);
        }

        void PopIY()
        {
			unsigned char lo,hi;
            SP++;
            hi = memory[SP];
            SP++;
            lo = memory[SP];
            
            IY = (unsigned short)(hi * 256 + lo);
        }

        void TakeBranch()
        {
			unsigned short disp=0;
            if (Immediate8 >= 128)
            {
                disp = (unsigned short)((255-Immediate8) + 1);
                PC -=  disp;
            }
            else
            {
                PC += Immediate8;
            }
        }

        void TakeJump()
        {
            PC = Immediate16;
        }


        unsigned char GetSource8()
        {
            if (mode == 0) return A;
            if (mode == 1) return B;
            if (mode == 2) return D;
            if (mode == 3) return E;
            if (mode == 4) return Immediate8;
            if (mode == 5) return memory[IX];
            if (mode == 6) return memory[Immediate16];
            if (mode == 7) return memory[IY];
            else
			{
				printf("Unknown mode %d. PC=%#04x", mode, PC);
				exit(0);
				return 0;
			}
        }

        unsigned char GetTarget8()
        {
            if (subType == 0) return A;
            else if (subType == 1) return B;
            else if (subType == 2) return D;
            else if (subType == 3) return E;
			else
			{
				printf("Unknown mode PC=%#04x\n.PC=%d", mode,PC);
				exit(0);
				return 0;
			}
        }

        void Inc()
        {
            /*A	000
            B	001 
            D	010 
            E	011
            IX	100
            IY	101
            SP	110
            */
            if (mode == 0)
            {
                A++;
                if (A == 0) ZeroFlag = 1;
                else ZeroFlag = 0;
            }
            else if (mode == 1)
            {
                B++;
                if (B == 0) ZeroFlag = 1;
                else ZeroFlag = 0;
            }
            else if (mode == 2)
            {
                D++;
                if (D == 0) ZeroFlag = 1;
                else ZeroFlag = 0;
            }
            else if (mode == 3)
            {
                E++;
                if (E == 0) ZeroFlag = 1;
                else ZeroFlag = 0;
            }
            else if (mode == 4)
            {
                memory[Immediate16]++;
                ZeroFlag = 0;
                if (memory[Immediate16] == 0) ZeroFlag = 1; 
            }
            else if (mode == 5)
            {
                IX++; 
            }
            else if (mode == 6)
            {
                SP++;
            }
            else if (mode == 7)
            {
                IY++;
            }
            
        }
        void Dec()
        {
            if (mode == 0)
            {
                A--;
                if (A == 0) ZeroFlag = 1;
                else ZeroFlag = 0;
            }
            else if (mode == 1)
            {
                B--;
                if (B == 0) ZeroFlag = 1;
                else ZeroFlag = 0;
            }
            else if (mode == 2)
            {
                D--;
                if (D == 0) ZeroFlag = 1;
                else ZeroFlag = 0;
            }
            else if (mode == 3)
            {
                E--;
                if (E == 0) ZeroFlag = 1;
                else ZeroFlag = 0;
            }
            else if (mode == 4)
            {
                memory[Immediate16]--;
                ZeroFlag = 0;
                if (memory[Immediate16] == 0)
                    ZeroFlag = 1;
            }
            else if (mode == 5)
            {
                IX--;
            }
            else if (mode == 6)
            {
                SP--;
            }
            else if (mode == 7)
            {
                IY--;
            }

        }
        /*ADD16	"add ix,a"	100	ADD	10	IX	000	A	144	0	1
ADD16	"add ix,b"	100	ADD	10	IX	001	B	145	0	1
ADD16	???	100	???	10		010		146	0	1
ADD16	???	100	???	10		011		147	0	1
ADD16	???	100	???	10		100		148	0	1
ADD16	add ix,ix	100	ADD	10	IX	101	IX	149	0	1
ADD16	add ix,nn	100	ADD	10	IX	110	IX	150	2	3
ADD16	add ix,iy	100	ADD	10	IX	111	IX	151	0	1*/
        void AddIX()
        {
            unsigned short data = GetAdd16Data(); 
            IX += data;
        }

        void AddIY()
        {
            unsigned short data = GetAdd16Data();
            IY += data;
        }

        unsigned short GetAdd16Data()
        {
            unsigned short data = 0;
            if (mode == 0)
                data = A;
            else if (mode == 1)
                data = B;
            else if (mode == 2)
                data = D;
            else if (mode == 3)
                data = E;
            else if (mode == 5)
                data = IX;
            else if (mode == 6)
                data = Immediate16;
            else if (mode == 7)
                data = IY;
            else
            {
				printf("Bad mode in add16. PC=%d\n", PC);
			}
            return data;
        }
        

        void Push() 
        {
            if (mode == 7) PushIY();
            else if (mode == 6) PushAll();
            else if (mode == 5) PushIX();
            else if (mode == 4) PushDE();
            else
            { 
                Push8();
            }
        }

        void Pop()
        {
            if (mode == 7) PopIY();
            else if (mode == 6) PopAll();
            else if (mode == 5) PopIX();
            else if (mode == 4) PopDE();
            else
            {
                PopReg8();
            }
 
        }


        void Push8()
        {
            unsigned char data = 0;
            if (mode == 0)
                data=A;
            else if (mode == 1)
                data = B;
            else if (mode == 2)
                data = D;
            else if (mode == 3)
                data = E;
            memory[SP] = data;
            SP--;
        }

        void PopReg8()
        {
            unsigned char data = Pop8();
            if (mode == 0)
                A = data;
            else if (mode == 1)
                B = data;
            else if (mode == 2)
                D = data;
            else if (mode == 3)
                E = data;

        }

        void Push8ToStack(unsigned char val)
        {
            memory[SP] = val;
            SP--;
        }

        unsigned char Pop8()
        {
            SP++;
            return memory[SP];
        }

        void PushDE()
        {
            Push8ToStack(E);
            Push8ToStack(D);
        }

        void PopDE()
        {
            D = Pop8();
            E = Pop8();
        }

        void PushIX()
        { 
            memory[SP] = (unsigned char)(IX % 256);
            SP--;
            memory[SP] = (unsigned char)(IX / 256);
            SP--;
        }

        void PushIY()
        {
            memory[SP] = (unsigned char)(IY % 256);
            SP--;
            memory[SP] = (unsigned char)(IY / 256);
            SP--;
        }

        void PushAll()
        {
            Push8ToStack(A);
            Push8ToStack(B);
            Push8ToStack(D);
            Push8ToStack(E);
            PushIX();
            PushIY();
        }

        void PopAll()
        {
            PopIY();
            PopIX();
            E = Pop8();
            D = Pop8();
            B = Pop8();
            A = Pop8();
        }

        /// <summary>
        /// Stores IX or IY
        /// </summary>
        void StoreIXIY()
        {
			unsigned char lo,hi;
			
            if (mode == 4)
            {//st ix,(iy) 
                memory[IY] = (unsigned char)(IX / 256);
                memory[IY + 1] = (unsigned char)(IX % 256);
            }
            else if (mode == 5)
            {//st ix,nn
                hi = (unsigned char)(IX / 256);
                memory[Immediate16] = hi;
                lo = (unsigned char)(IX % 256);
                memory[Immediate16+1] = lo;
            }
            else if (mode == 6)
            {//st iy,(ix) 
                memory[IX] = (unsigned char)(IY/256);
                memory[IX+1] = (unsigned char)(IY%256);
            }
            else if (mode == 7)
            {//st iy,nn
                hi = (unsigned char)(IY / 256);
                memory[Immediate16] = hi;
                lo = (unsigned char)(IY % 256);
                memory[Immediate16 + 1] = lo;
            }
        }

        void LoadIndexed()
        {
            unsigned short addr = Immediate8;
            if (mode == 5)
               addr = (unsigned short)(Immediate8 + IX);
            else if (mode ==7)
                addr = (unsigned short)(Immediate8 + IY);

            A = memory[addr];
            if (A == 0) ZeroFlag = 1;
            else ZeroFlag = 0;
        }

        void CompareIndexed()
        {
            unsigned short addr = Immediate8;
			unsigned char data =0;
            if (mode == 5)
                addr = (unsigned short)(Immediate8 + IX);
            else if (mode == 7)
                addr = (unsigned short)(Immediate8 + IY);

            data = memory[addr];

            SetFlags(A,data);
        }


        unsigned short GetLoad16Data()
        {
			unsigned char lo,hi;
			
            if (mode == 0)
            {
                return Immediate16;
            }
            else if (mode == 1)
            {//(NN)
                hi = memory[Immediate16];
                lo = memory[Immediate16 + 1];
                return (unsigned short)(hi * 256 + lo);
            }
            else if (mode == 2)
            {//(IX)
                hi = memory[IX]; 
                lo = memory[IX+1];
                return (unsigned short)(hi * 256 + lo);
            }
            else if (mode == 3)
            {//(IY)
                hi = memory[IY];
                lo = memory[IY + 1];
                return (unsigned short)(hi * 256 + lo);
            }
            else if (mode == 4)
            {//SP
                return SP;
            }
            else if (mode == 5)
            {//IX
                return IX;
            }
            else if (mode == 6)
            {//NN
                return Immediate16;
            }
            else if (mode == 7)
            {//IY
                return IY;
            }
			RageDump();
			exit(0);
			return 0;		
        }

        void LoadIX()
        {
            if (mode == 0)
                SP = GetLoad16Data();
            else
                IX = GetLoad16Data();
        }

        void LoadIY()
        {
            if (mode == 0)
                SP = GetLoad16Data();
            else
                IY = GetLoad16Data();
        }

        void LoadSP()
        {
            SP = Immediate16;
        }

        void SpecialOps1()
        {
            if (mode == 0) GetObjAttr16();
            else if (mode == 1) SetObjAttr16();
            else if (mode == 2) PrintStrPtr();
            else if (mode == 3) PrintStrPtrCr();
            else if (mode == 4) PrintStrN16();
            else if (mode == 5) PrintStrN16Cr();
            else if (mode == 6) Newline();
            else if (mode == 7) RMod();
        }
        /*
         * Special	RESTORE	111		11			000			248	0	1
Special	SETATTR	111		11			001			249	0	1
Special	SETPROP	111		11			010			250	0	1
Special	GETATTR	111		11			011			251	0	1
Special	GETPROP	111		11			100			252	0	1
Special	GETPARENT	111		11			101			253	0	1
Special	SETENVVAR	111		11			110			254	0	1
Special		111		11			111			255	0	1*/
        void SpecialOps2()
        {
            if (mode == 0) ReadLine();
            else if (mode == 1) CharOut();
            //else if (mode == 2) SetObjProp();
            else if (mode == 3) Streq();
            else if (mode == 4) AnyKey();
            else if (mode == 5) CLS();
            else if (mode == 6) Status();
            else if (mode == 7) Save();
        }

        void SpecialOps3()
        {
            if (mode == 0) Restore();
            else if (mode == 1) SetObjAttr();
            else if (mode == 2) SetObjProp();
            else if (mode == 3) GetObjAttr();
            else if (mode == 4) GetObjProp();
            else if (mode == 5) GetParent();
            else if (mode == 6) SetEnv();
            else if (mode == 7) Quit();
        }

         
        
		void ReadLine()
		{    
			int len = 0;
			int i = 0;
			 
			 
			//gets(buffer);
			//fgets(buffer, 40, stdin);
			size_t size=40;
			 
			memset(buffer, 0, 40);
			//gets(buffer);
			fflush (stdin);
			fgets(buffer, 40, stdin);
		//	printf("OK\n");
			len = strlen(buffer);
			buffer[len - 1] = 0;

			len = strlen(buffer);
			 
			for (;  i < min(len,40); i++)
			{
				memory[IX + i] = (unsigned char)buffer[i];
			}
			
			//null terminate buffer
			//WriteByte((unsigned short)(IX + i) ,0);
			memory[IX + i]=0;
		//printf("Buffer=Buffer=%s\n",&memory[IX]);
		//	printf("OK\n");
		}

			 

        void AnyKey()
        {
			fflush(stdin);
			fgetc(stdin);
        }

        void Streq()
        {
            int i = 0;
            while (1)
            {
                char ch1 = (char)memory[IX + i];
                char ch2 = (char)memory[IY + i];

                if (ucase(ch1) != ucase(ch2))
                {
                    A = 0;
                    return;
                }
                //chars are equal
                if (memory[IX + i] == 0) break;
                i++;
            }
            A = 1;
        }


        /// <summary>
        /// Prints the string at the address referenced by IX
        /// </summary>
        void PrintStrPtr()
        {
            unsigned short i = IX;
			int len=0;
            while (memory[i] != 0)
            {
                len = WordLenIX(i);
                if (len > (ScreenWidth - hPos))
                {
                    Newline();
                    hPos = 0;
                }
                
                //print the word
                i = PrintWord(i); 

                if (memory[i] == 0)
                    break;

                ChOut((unsigned char)' ');
                hPos++;
                i = MoveNextWord(i);
                
            }
        }
        /// <summary>
        /// skips white space and move to the next letter
        /// </summary>
        /// <returns></returns>
        unsigned short MoveNextWord(unsigned short i)
        {
            while (memory[i] == (unsigned char)' ')
            {
                i++;
            }
            return i;
        }

        unsigned short PrintWord(unsigned short i)
        {
            while (memory[i] != 0 && memory[i] != ' ')
            {
                ChOut(memory[i]);
                hPos++;
                i++;
            }
            return i;
        }


        /// <summary>
        /// iy = id#
        /// </summary>
        void PrintStrN16()
        {
			int i = 0;
            unsigned short ix = IX;
            unsigned short addr = StringTable;
            for (; i < ix; i++)
            {
                int len = memory[addr];
                addr += (unsigned short)len;
                addr += 2; //skip length and null
            }
            addr++; //skip length unsigned char
            //ix contains addr
            IX = addr;
            PrintStrPtr();
            IX = ix;
        }

        void PrintStrN16Cr()
        {
            PrintStrN16();
            Newline();
        }

        void PrintStrPtrCr()
        {
            PrintStrPtr();
            Newline();
        }

        void CLS()
        {
			printf("\x1b[2J"); //cls
			printf("\x1b[2;0H"); //position cursor
        }


        void SetEnv()
        {

            if (A == 1)
            {
                StringTable = IX;
            }
            else if (A == 2)
            {
                ObjectTable = IX;
            }
            else if (A == 3)
            {
                outputChannel = CONSOLE;
            }
            else if (A == 4)
            {
                outputChannel = IXREGISTER;
                bufferAddr = IX;
                bufferStart = IX;
            }
        }

		//ix = start address
		//iy = end address
        void Save()
		{
			FILE *fp = 0;
			int len = 0;
			
			printf("Enter save file name (without extension)\n");
			
			memset(buffer, 0, sizeof(char));
			
			//gets_s(buffer, 80);
			//gets(buffer);
			fgets(buffer, 40,stdin);
			len = strlen(buffer);

			buffer[len - 1] = 0;
			strcat(buffer,".sav");
			fp = fopen(buffer, "wb");
			fwrite(&memory[IX], 1, IY-IX, fp);
			fclose(fp);
	
			printf("Game saved.\n");
		}
        

		//IX contains starting address to put file data
        void Restore()
		{
			FILE *fp = 0;
			int fileSize=0;
			int len;

			printf("Enter save file name (without extension)\n");
			memset(buffer, 0, sizeof(char));
			//gets(buffer);
			fgets(buffer, 40, stdin);
			len = strlen(buffer);
			buffer[len - 1] = 0;
			strcat(buffer,".sav");
			
			fp = fopen(buffer, "rb");
			if (fp != 0)
			{
				printf("getting file size...\n");
				fseek(fp, 0, SEEK_END);
				fileSize = ftell(fp);
				rewind(fp);
				printf("file size was %d bytes\n", fileSize);				
				fread(&memory[IX], 1, fileSize, fp);
				fclose(fp);
			}
			else
			{
				printf("File not found.\n");
			}
		}
       

        void RMod()
        {
			int r = rand();
            A = (unsigned char)(r % B);
        }

        void Quit()
        {
			exit(0);
        }


        void CharOut()
        {
            if (A == ' ')
            {
                if (hPos == ScreenWidth - 1)
                {
                    Newline();
                }
                else
                {
                    ChOut(A);
                    hPos++;
                }
            }
            else
            {
                ChOut(A);
                hPos++;
            }
        }

        void Newline()
        {
            hPos = 0;
            printf("\n"); 
        }

        /// <summary>
        /// scans up to null or space
        /// </summary>
        /// <param name="i"></param>
        /// <returns></returns>
        int WordLenIX(unsigned short i)
        {
            int count = 0;
         
            while (memory[i]!=0 && memory[i] != (unsigned char)' ')
            {
                count++;
                i++;
            }
            return count;
        }

        //ix = table
        //d = object
        //e = attr
        //sets A
        void GetObjAttr()
        {
            unsigned short addr = (unsigned short)(ObjectTable + D * OBJECT_ENTRY_SIZE + E);
            A = memory[addr];
        }

        void GetObjAttr16()
        {
            unsigned short addr = (unsigned short)(ObjectTable + D * OBJECT_ENTRY_SIZE + E);
            IX = (unsigned short)(memory[addr + 1] * 256 + memory[addr]); //flip unsigned chars
        }

        void SetObjAttr16()
        {
            unsigned short addr = (unsigned short)(ObjectTable + D * OBJECT_ENTRY_SIZE + E);
            memory[addr] = (unsigned char)(IX % 256);
            memory[addr+1] = (unsigned char)(IX / 256);
        }

        //ix = table
        //d = object
        //e = prop # 1 - 16
        //sets A
        void GetObjProp()
        {
			unsigned short b=0;
			unsigned short mask=0;
            unsigned short addr = (unsigned short)(ObjectTable + D * OBJECT_ENTRY_SIZE);
            addr += PROPERTY_BYTE_1;
            if (E > 8)
            {
                addr++; //go to next property unsigned char
            }

            b = (unsigned short) memory[addr];
            mask = masks[E];
            ZeroFlag = 0;
            A = 1;

            if (  (b & mask) == 0)
            {
                ZeroFlag = 1;
                A = 0;
            }
        }

        //ix = table
        //b = object
        //d = attr
        //e = value 1- 16
        void SetObjProp()
        {
            unsigned short addr = (unsigned short)(ObjectTable + B * OBJECT_ENTRY_SIZE + PROPERTY_BYTE_1);
			unsigned char mask;
			unsigned char b;
			unsigned char temp;
            if (D > 8)
            {
                addr++;
            }
            if (E==0)
            {//clear it
                mask = (unsigned char) masks[D];
                mask = (unsigned char)~mask;
                b = (unsigned char)(memory[addr] & mask);
                memory[addr] = b; 
            }
            else
            {//set it
                b = memory[addr];
                temp = (unsigned char)( b | masks[D] );
                memory[addr] = temp;
            }
         }


        //ix = table
        //b = object
        //d = attr
        //e = value
        void SetObjAttr()
        {
            unsigned short addr = (unsigned short)(ObjectTable + B * OBJECT_ENTRY_SIZE + D);
            memory[addr] = E;
        }

		//parent of A in A
        void GetParent()
        {
            unsigned short addr = (unsigned short)(ObjectTable + A * OBJECT_ENTRY_SIZE + 1);
            A = memory[addr];
        }
		
		//ix contains room c
		//iy contains score
		void Status()
        {
			int len;
			int i=0;
			printf("\x1b[s"); //save cursor
			printf("\x1b[0;0H"); //home

			sprintf(topline,"##### %s ", (char*)&memory[IX] );
			len = strlen(topline);
			for (i=len; i < SCORE_INDENT; i++)
			{
				strcat(topline,"#");
			}
			strcat(topline," SCORE:");
 
			strcat(topline,(char*)&memory[IY]);
			strcat(topline,"/100");


			for (i=strlen(topline); i < SCREEN_WIDTH; i++)
			{
				strcat(topline,"#");
			}


			printf(topline);
			printf("\x1b[u"); //restore cursor
			 
        }

 	
        void ChOut(unsigned char ch)
        {
            if (outputChannel == CONSOLE)
            {
                printf("%c", ch);
            }
            else if (outputChannel == IXREGISTER)
            {
                memory[bufferAddr] = ch;
                bufferAddr++;
            }
            else
            {
                printf("Bad output channel. PC=%d",PC);
				exit(0);
            }
        } 
			
		int cfileexists(const char * filename)
		{
			// try to open file to read 
			FILE *file;
			if (file = fopen(filename, "r"))
			{
				fclose(file);
				return 1;
			}
			return 0;
		}

/*
		int min(int x, int y)
		{
			if (x < y) return x;
			return y;
		}
	*/	 
		unsigned char ucase(unsigned char ch)
		{
			if (ch >=  'a' && ch <='z') 
			{
				return ch-32;
			}
			return ch;
		}
		
		void GetDbgCmd()
		{
			int i=0;
			while (1)
			{ 
				fflush(stdin);
				printf(":");
				fgets(cmdbuf,40,stdin);
				if (cmdbuf[0] == 0 || cmdbuf[0] == 's')
				{
					step=1;
					break;
				}
				if (strcmp(cmdbuf,"r")==0)
				{
					step = 0;
					break;
				}
				else if (cmdbuf[0] == 'r' && cmdbuf[1] != 0)
				{
					breakPoints[numBp] =GetHex(&cmdbuf[1]);
					numBp++;
					printf("breakpoint %#04x set.\n",breakPoints[numBp-1]);
					step = 0;
					break;
				}
				else if (cmdbuf[0] == 'b')
				{
					breakPoints[numBp] =GetHex(&cmdbuf[1]);
					numBp++;
					printf("breakpoint %#04x set.\n",breakPoints[numBp-1]);
					step=0;
				}
				else if (cmdbuf[0] == 'x')
				{
					unsigned short addr =GetHex(&cmdbuf[1]);
					for (i=0; i < 16; i++)
					{
						printf("%#02x ",memory[addr]);
						addr++;
					}
					printf("\n");
 				}
				else
				{
					printf("invalid command.\n");
				}
				fflush(stdin);
			}
			
			
		}
		
		void RageDump()
		{
			printf("PC=%#04x A=%#02x B=%#02x D=%#02x E=%#02x IX=%#04x IY=%#04x\n",
			PC,A,B,D,E,IX,IY);
		}
		
		unsigned short GetHex(char *buf)
		{
			unsigned short total =0;
			unsigned int pow=1;
			int i=3;
			for (; i >=0; i--)
			{
				char ch=buf[i];
				if (ch >='0' && ch <= '9')
				{
					ch=ch-'0';
				}
				else if (ch >='A' && ch <= 'F')
				{
					ch=ch-'A' + 10;
				}
				else if (ch >='a' && ch <= 'f')
				{
					ch=ch-'a' + 10;
				}
				total += ch*pow;
				pow *= 16;
			}
			return total;
		}
int min(int a, int b)
{
 if (a > b) return a;
 return b;
}
