{--------------------------------------------------------------------------}
{                      UNIVERSITY OF TECHNOLOGY                            }
{                   MAJOR OF INFORMATIC TECHNOLOGY                         }
{                    PROGRAM PLAYING MUSIC KARAOKE                         }
{                      Written by : NGUYEN NGOC VAN                        }
{                    Student code : 00DTH201                               }
{                           Class : 00DTH01                                }
{                     Last update : 07/10/2000                             }
{--------------------------------------------------------------------------}
{                     Environment : Borland Pascal ver 7.0                 }
{                     Source file : KARAOKE.PAS                            }
{                     Mode memory : Small                                  }
{                         Compile : BPC KARAOKE.PAS                        }
{                            Runs : KARAOKE                                }
{                    Support file : VN.COM                                 }
{--------------------------------------------------------------------------}

{$M 15000,0,15000} { bytes 15000 for stack, byte 15000 for heap}
PROGRAM KARAOKE;
USES Crt, Dos, Graph;

CONST MaxItem = 8;
TYPE Items = ARRAY[1..MaxItem] OF STRING[30];

CONST
  Maxnot = 1000;
  WidthLen : Byte = 10; { Do nhanh cham cua bai hat Default }
  x1 = 14;
  x2 = 66;
  Tg = 70;              { Dieu chinh do nhanh cham cua dong quang cao }
  y1p = 6;
  y2p : Byte = 6;
  Adver : STRING = '............................................................'+
  '...................<< PROGRAM KARAOKE >> Version 2.0 Copyright (c) By Student'+
  ' NGUYEN NGOC VAN At October 2000 All Rights Reserved';
  { Cac ma phim dieu kien }
  Up = #72;
  Down = #80;
  Left = #75;
  Right = #77;
  Esc = #27;
  Enter = #13;
  Home = #71;
  EndKey = #79;
  Max = 72;   { Numbers Files.Not }
  { Cac chuc nang cua chuong trinh }
  Bar  : Items = ('Ch¥¤ng tr×nh','H¥íng d¸n','B¨n quyÒn','Thoªt','','','','');
  Pop1 : Items = ('Ch¥¤ng tr×nh KaraOke...','Giíi thiÖu ch¥¤ng tr×nh', 'N«p fonts tiÕng viÖt','','','','','');
  Pop2 : Items = ('Cªc th£ng tin','H¥íng d¸n sö dông','Tæ chøc ch¥¤ng tr×nh', '','','','','');
  Pop3 : Items = ('Môc khªi quªt','H¥íng phªt triÓn','Tãm t¯t th£ng tin','','','','','');

TYPE
  Str40 = STRING[40]; { Chuoi gom 40 ky tu }
  Str31 = STRING[31]; { Chuoi gom 31 ky tu }
  Str63 = STRING[63]; { Chuoi gom 63 ky tu }
  Str80 = STRING[80]; { Chuoi gom 80 ky tu }
{-------------------------------------------------------------------------}
  Records = RECORD
               HightL,WidthL : Word;  { Cao do, truong do }
               CharL         : Str40; { Loi bai hat }
            END;
  ArrRecs=ARRAY[1..Maxnot] OF Records;
{-------------------------------------------------------------------------}
  ArrStr=ARRAY[1..Max] OF Str31; { Do lon cua bang }
  ArrChar=ARRAY[1..8] OF Byte ;  { Bieu dien Bit }
{-------------------------------------------------------------------------}
  ElementAllScreen = RECORD
                        CASE Byte OF
                           0 : (Point      : Word);
                           1 : (ASCII,Attr : Byte)
                        END;
  AllScreen=ARRAY[1..2000] OF ElementAllScreen;
{-------------------------------------------------------------------------}
  PartScreen=^ElementPartScreen;
  ElementPartScreen = RECORD
                         Element : ElementAllScreen;
                         Next    : PartScreen
                      END;
{-------------------------------------------------------------------------}

VAR
  Monitor : AllScreen; { Luu man hinh vao bien Monitor }
  Songs   : ArrRecs;   { Bai hat }
  K       : Word;      { Dem bai hat }
  Ch      : Char;      { Phim de hat nhac }
  Fn      : Str31;     { Lay duong dan }
  Stop    : Boolean;   { Dung hay khong }
  OptBar,              { Thanh Menu ngang }
  OptPop,              { Thanh Menu doc }
  Key     : Byte;
  TableChar  : ARRAY[0..255] OF ArrChar ABSOLUTE $F000:$FA6E;
  IsRegister : ShortInt;
  { Buoc CPU cap phat vung nho cho TableChar tai dia chi $F000:$FA6E }

{----------------------------------------------------------------------}
{      CAC HAM LAY MODE MAN HINH, CON TRO, XU LY CHUOI VA BAI HAT      }
{----------------------------------------------------------------------}

{ Xac dinh dia chi cua man hinh }
FUNCTION SegmentVideo : Word;
VAR Address : Word;
  BEGIN
    Address:=MemW[$40:$63];
    IF Address=$3D4 THEN SegmentVideo:=$B800; { Dia chi CGA }
    IF Address=$3B4 THEN SegmentVideo:=$B000  { Dia chi MonoChome }
  END;

{ Ham chen mot ky tu voi Times lan }
FUNCTION Replicate(Symbol : Char; Times : Byte) : Str80;
VAR
  i    : Byte;
  Temp : str80;
  BEGIN
    Temp:='';
    FOR i:=1 TO Times DO
      Temp:=Temp+ Symbol;
      Replicate:=Temp
  END;

{ Ham xac dinh dia chi cua cac dong va cot }
FUNCTION OffSet(Col, Row : Byte) : Word;
  BEGIN
    CASE Mem[$0000:$0449] OF
      0,1 : OffSet:=2*(Col-1)+80*(Row-1); { Dia chi cua man hinh 40x25 }
    ELSE
      Offset:=2*(Col-1)+160*(Row-1)       { Dia chi cua man hinh 80X25 }
    END
  END;

{ Lay lai mau nen hien tai }
FUNCTION GetBkColor : Byte;
  BEGIN
    GetBkColor:=TextAttr DIV 16
  END;

{ Lay lai mau chu hien tai }
FUNCTION GetTextColor : Byte;
  BEGIN
    GetTextColor:=TextAttr MOD 16
  END;

{ Tinh chieu dai lon nhat cua chuoi }
FUNCTION MaxLength(St : ArrStr; NumChar : Byte) : Byte;
VAR Max,i:byte;
  BEGIN
    Max:=1;
    FOR i:=1 to NumChar DO
    IF Length(St[i]) > Max THEN Max:=Length(St[i]);
    MaxLength:=Max
  END;

{ Tim cac file.not trong mot duong dan }
FUNCTION FindFiles(Path : Str31; Attr : Byte; VAR ArrDir : ArrStr; VAR Count : Byte) : Boolean;
VAR DirBuff : SearchRec;
  BEGIN
    Count:=1;
    FindFirst(Path,Attr,DirBuff);
    IF DosError=0 THEN
      BEGIN
        FindFiles:=TRUE;
        REPEAT
          ArrDir[Count]:=DirBuff.Name;
          Inc(Count);
          FindNext(DirBuff)
        UNTIL (DosError<>0) OR (Count=Max+1);
        Dec(Count)
      END
    ELSE FindFiles:=FALSE
  END;

{ Khoi tao danh sach cac FILE nhac }
FUNCTION ListSongs(Path : Str31; VAR NumSongs : ArrStr; VAR NumOff : Byte) : Boolean;
  BEGIN
    ListSongs:=FindFiles(Path, $3F, NumSongs, NumOff)
  END;

{----------------------------------------------------------------------}
{                HAM CHO RA SO NHO NHAT TRONG HAI SO                   }
{----------------------------------------------------------------------}
FUNCTION Minium(A,B :Integer) : Integer;
BEGIN
  IF A<B THEN Minium:=A ELSE Minium:=B
END;

{----------------------------------------------------------------------}
{               HAM CAT HET KHOANG TRONG TRONG CHUOI                   }
{----------------------------------------------------------------------}
FUNCTION AllTrim(Str : Str80) : Str80;
VAR i : Byte;
BEGIN
  WHILE Pos(#32#32,Str) > 0 DO Delete(Str,Pos(#32#32,Str),1);
  WHILE Str[1] = #32 DO Delete(Str,1,1);
  WHILE Str[Length(Str)] = #32 DO Delete(Str,Length(Str),1);
  AllTrim:=Str
END;

{ Kiem tra xem co dang ky chuong trinh chua }
FUNCTION CheckRegister : Boolean;
VAR Fp   : FILE OF ShortInt;
    Temp : ShortInt;
    Size : LongInt;
  BEGIN
     {$I-}
     Assign(Fp,'C:\WINDOWS\register.dat');
     Reset(Fp);
     {$I+}
     IF IOResult <> 0 THEN
        BEGIN
           Clrscr;
           Writeln('Cannot open file Register.dat, press any key to exit program');
           REPEAT UNTIL KeyPressed;
           Halt(1);
        END;
     Size := FileSize(Fp);
     Seek(Fp, Size - 1);
     Read(Fp, Temp);
     IF Temp = 1 THEN CheckRegister := TRUE
     ELSE CheckRegister := FALSE;
     Close(Fp);
  END;

{ Ham lay ky tu o dia }
FUNCTION GetDisk : Str31;
VAR
  Fp  : FILE OF Char;
  Tmp : STRING[3];
BEGIN
  {$I-}
  Assign(Fp,'C:\WINDOWS\register.dat');
  Reset(Fp);
  {$I+}
  IF IOResult <> 0 THEN
    BEGIN
      Clrscr;
      Writeln('Cannot open file register.dat, press any key to exit program');
      REPEAT UNTIL KeyPressed;
      Halt(1);
    END;
  IF CheckRegister = TRUE THEN Seek(Fp,FileSize(Fp) - 5)
  ELSE Seek(Fp,FileSize(Fp) - 4);
  Read(Fp,Tmp[1],Tmp[2],Tmp[3]);
  Tmp := Tmp[1]+Tmp[2]+Tmp[3];
  GetDisk := Tmp;
  Close(Fp);
END;

{----------------------------------------------------------------------}
{          CAC MODULE NAP FONT GIAI MA VA GIAI PHONG BO NHO            }
{----------------------------------------------------------------------}

{ Nap FONT tieng viet }
PROCEDURE LoadFont(Ch : Char);
VAR St : STRING[28];
  BEGIN
    CASE Ch OF
     '1' : St:='/c '+GetDisk+'TOPICS\KARAOKE\VN';
     '0' : St:='/c '+GetDisk+'TOPICS\KARAOKE\VN OFF';
    END;
    SwapVectors;
    Exec(Getenv('COMSPEC'),St);
    SwapVectors;
  END;

{---------------------------------------}
{  Ham giai phong chuong trinh TOPICS   }
{  Vao : Khong                          }
{  Ra  : Khong                          }
{---------------------------------------}
PROCEDURE ReleaseFont;
VAR St : STRING[26];
  BEGIN
    St := '/c C:\WINDOWS\deltopic.com';{ Goi chuong trinh ngoai }
    SwapVectors;                       { Doi ngat vector }
    Exec(Getenv('Comspec'),St);        { Nap tap tin giai ma tieng viet }
    SwapVectors                        { Doi lai ngat vector }
  END;

{----------------------------------------------------------------------}
{             CAC MODULE XU LY THUOC TINH MAU MAN HINH                 }
{----------------------------------------------------------------------}

{ Ham bat mau nen sang hon }
PROCEDURE DoBlinking(Blinking : Boolean);
VAR Regs : Registers;
BEGIN
  Regs.AH := $10;
  Regs.AL := $03;
  IF Blinking THEN Regs.BL := 1
  ELSE Regs.BL := 0;
  Intr($10,Regs);
END;

{ Ham khoi dong driver cua chuot }
FUNCTION InitMouse : Boolean;
VAR
  Regs : Registers;
BEGIN
  Regs.AX := 0;
  Intr($33, Regs);
  InitMouse := Regs.AX <> 0;
  Regs.AX := $0001;
  Intr($33, Regs);
END;

{ Xac dinh trang thai cac nut bam }
FUNCTION ClickMouse(VAR Col, Row : Byte) : Integer;
VAR Regs : Registers;
BEGIN
  Regs.AX := $0003;
  Intr($33, Regs);
  Row := Regs.DX SHR 3 + 1;
  Col := Regs.CX SHR 3 + 1;
  ClickMouse := Regs.BX;
END;

{ Dau con tro chuot }
PROCEDURE HideMouse;
VAR Regs : Registers;
BEGIN
  Regs.AX := $0002;
  Intr($33, Regs);
END;

{ Hien con tro chuot }
PROCEDURE ShowMouse;
VAR Regs : Registers;
BEGIN
  Regs.AX := $0001;
  Intr($33, Regs);
END;

{ Thiet lap vung hoat dong cua chuot }
PROCEDURE SetPMCL;
VAR Regs : Registers;
BEGIN
  Regs.AX := $0007;
  Regs.CX := 0;
  Regs.DX := 2*320 - 8;
  Intr($33, Regs);
  Regs.AX := $0008;
  Regs.CX := 0;
  Regs.DX := 200 - 8;
  Intr($33, Regs);
  Regs.AX := $010D;
  Regs.BX := 0;
  Intr($33, Regs);
END;

{ Duy chuyen con tro chuot }
PROCEDURE MoveMouse(x, y : Byte);
VAR Regs : Registers;
BEGIN
  Regs.AX := $0004;
  Regs.CX := 8*x - 1;
  Regs.DX := 8*y - 1;
  Intr($33, Regs);
END;

{ Dong driver chuot }
PROCEDURE CloseMouse;
VAR Regs : Registers;
BEGIN
  HideMouse;
  Regs.AX := 0;
  Intr($33, Regs);
END;

{ Thu tuc thiet lap mau chu va mau nen }
PROCEDURE SetTextColor(BkColor,TxColor : Byte);
  BEGIN
    TextAttr:=(BkColor SHL 4) + TxColor;
  END;

{ Thiet lap mau vung bien man hinh }
PROCEDURE SetBorderColor(Color : Byte);
VAR Regs : Registers;
  BEGIN
    Regs.AH:=$10;
    Regs.AL:=1;
    Regs.BH:= Color AND 63;
    Intr($10,Regs)
  END;

{ Thu tuc thay doi thuoc tinh cua mot vung man hinh
  voi cac toa do goc tren ben trai la (x1,y1) va goc
  duoi ben phai la (x2,y2) voi mau nen la Bk }
PROCEDURE SetAttrib(x1, y1, x2, y2, Tx, Bk : Byte);
VAR
  Col,Row,Attr :Byte;
  BEGIN
    Attr:=(Bk SHL 4)+Tx;
    FOR Col:=x1 TO x2 DO
      FOR Row:=y1 TO y2 DO
        Mem[SegmentVideo:OffSet(Col,Row)+1]:=Attr
        { Bo nho cua man hinh bat dau tu Offset $B800:3E7F }
        { SegmentVideo=$B800 la dia chi cua man hinh CGA }
        { Dia chi nay thay doi tuy theo man hinh cua ban }
  END;

{ Ha thap Mode man hinh }
PROCEDURE LowVideoWindow(x1, y1, x2, y2 : Byte);
VAR i,j,Address : Byte;
  BEGIN
    FOR i:=y1 TO y2 DO
      FOR j:=x1 TO x2 DO
        BEGIN
          Address:=Mem[SegmentVideo:Offset(j,i)+1];
          Address:=Address MOD 16;
          CASE Address OF
            0..8  : Mem[SegmentVideo:Offset(j,i)+1]:=$08;
            9..15 : Mem[SegmentVideo:Offset(j,i)+1]:=Address-8
          END
        END
  END;

{ Tao vung bong cho cac textbox }
PROCEDURE Shape(x1, y1, x2, y2 : Byte);
VAR i,t : Byte;
  BEGIN
    LowVideoWindow(x1,y2,x2-2,y2);
    LowVideoWindow(x2-1,y1,x2,y2)
  END;

{ Thu tuc xoa mot phan man hinh voi mau BackGround }
PROCEDURE ClearPartScreen(x1, y1, x2, y2, BackGround : Byte);
  BEGIN
    Window(x1,y1,x2,y2);
    TextBackGround(BackGround);
    ClrScr;
    Window(1,1,80,25)
  END;

{----------------------------------------------------------------------}
{        CAC MODULE XU LY CHUOI VA CAC KY TU DE DUA RA MAN HINH        }
{----------------------------------------------------------------------}

{ Viet chuoi Message tai toa do X,Y }
PROCEDURE WriteXY(x, y : Byte; Message : Str80);
   BEGIN
     GotoXY(x,y);
     Write(Message)
   END;

{ Viet chuoi St tai toa do X,Y va co mau }
PROCEDURE WriteColorXY(x, y, BkColor, TxColor : Byte; St : Str80);
  BEGIN
    SetTextColor(BkColor,TxColor);
    WriteXY(x,y,St)
  END;

{ Viet mot chuoi tai toa do x,y co mau
  chu la Tx va mau nen la Bk }
PROCEDURE SayXY(x, y, Bk, Tx : Byte; Message : Str80);
VAR
  OldTx,OldBk,Len : Byte;
  BEGIN
    OldTx:=GetTextColor; { Lay lai mau chu hien tai}
    OldBk:=GetBkColor; { Lay lai mau nen hienntai }
    SetTextColor(Bk,Tx);
    Len:=Ord(Message[0])+2;
    GotoXY(x,y);
    Write('':Len);
    WriteXY(x+1,y,Message);
    SetTextColor(OldBk,OldTx)
    { Tra lai cac thong so mau mac nhien ban dau }
  END;

{ Ham viet mot chuoi tai toa do x, y. Xam nhap vao video RAM }
PROCEDURE WriteVRAM(x, y, Attr : Byte; StrOut : Str80);
VAR
  VideoOffset : Word;
  i           : Byte;
BEGIN
    VideoOffset:=Offset(x,y);
    FOR i:=1 TO Length(StrOut) DO
      BEGIN
        MemW[SegmentVideo:VideoOffset]:=(Attr SHL 8) + Ord(StrOut[i]);
        Inc(VideoOffset,2);
      END
END;

{ Thu tuc viet mot ky tu tai toa do x,y co mau la
  Attr va do dai la Len. }
PROCEDURE WriteChar(x, y, Attr, Len : Byte; Ch : Char);
VAR
  VideoOffset : Word;
  i           : Byte;
BEGIN
    VideoOffset:=Offset(x,y);
    FOR i:=1 TO Len DO
      BEGIN
        MemW[SegmentVideo:VideoOffset]:=(Attr SHL 8) + Ord(Ch);
        Inc(VideoOffset,2);
      END
  END;

PROCEDURE Button(x, y, AttrText, AttrLetter, BkColor : Byte; Style : Boolean; Msg : Str80);
CONST CodeStyle : ARRAY[1..4] OF Byte = (16, 17, 223, 220);
VAR
   Len, i, j, k : Byte;
   TypeAttr  : Word;
   Flag : Boolean;
BEGIN
  Flag := FALSE;
  i := 1;
  Len := Length(Msg);
  TypeAttr := AttrText SHL 4;
  WHILE (i <= Len) AND NOT Flag DO
    BEGIN
      IF Msg[i] = Chr(126) THEN Flag := TRUE
      ELSE Inc(i);
    END;
  IF Flag THEN
    BEGIN
       Msg := Copy(Msg, 1, i - 1) + Copy(Msg, i + 1, Len - i);
       WriteVRAM(x, y, Attrtext, Msg);
       WriteChar(x + i - 1, y, AttrLetter, 1, Msg[i]);
    END
  ELSE WriteVRAM(x, y, AttrText, Msg);
  WriteChar(x + 1, y + 1, BkColor SHL 4, Len - 1, Chr(CodeStyle[3]));
  WriteChar(x + Len - 1, y, BkColor SHL 4, 1, Chr(CodeStyle[4]));
  IF Style THEN
    BEGIN
      WriteChar(x,y,AttrText,1,Chr(CodeStyle[1]));
      WriteChar(x+Len-2,y,AttrText,1,Chr(CodeStyle[2]));
    END
END;

{ Thuc tuc lam day mot khung co toa do x1,y1,x2,y2
  bang mot ky tu bat ky. Thuong dung de trang tri }
PROCEDURE FillCharFrame(x1, y1, x2, y2, Attr : Byte; Ch : Char);
VAR
  Y : Byte;
  BEGIN
    FOR y:=y1 TO y2 DO
      WriteChar(x1,y,Attr,x2-x1+1,Ch);
  END;

{ Thu tuc dat mot chuoi co cac ky tu chay tren man hinh
  tai toa do X,Y.Tham so Mode cho biet chuoi chay sang
  ben trai hay sang ben phai, tham so Delaytimes cho
  biet do nhanh cham cua chuoi chay }
PROCEDURE PutString(VAR St : STRING; x,y,Mode,DelayTimes : Integer);
VAR
  Tempo : Char;
  i     : Integer;
   BEGIN
     IF (Mode=1) THEN  { Cho chu chay tu phai sang trai }
       BEGIN
         Tempo:=St[1];
         FOR i:=1 TO Length(St)-1 DO St[i]:=St[i+1];
         St[Length(St)]:=Tempo
       END
     ELSE              { Cho chu chay tu trai sang phai }
       BEGIN
         Tempo:=St[Length(St)];
         FOR i:=Length(St) DOWNTO 2 DO St[i]:=St[i-1];
         St[1]:=Tempo
       END;
     SetTextColor(Red,White);
     WriteXY(X,Y,St);
     Delay(DelayTimes)
   END;

{----------------------------------------------------------------------}
{              THU TUC TAO RA DONG CHU CHAY TREN MAN HINH              }
{----------------------------------------------------------------------}
PROCEDURE SwapChar(St : STRING);
VAR
  K   : Integer;
  Tin : STRING;
  BEGIN
    SetAttrib(1,25,80,25,$1,$1);
    Tin:=Replicate(' ',78)+' '+St+' ';
    FOR k:=1 To Length(Tin) DO
      BEGIN
        TextColor(Yellow);
        WriteXY(2,25,Copy(Tin,K,Minium(78,Length(Tin)-K+1)));
        Delay(100);
        IF KeyPressed THEN Exit;
      END
  END;

{----------------------------------------------------------------------}
{              CAC MODULE XU LY CAC KHUNG HOP THOAI                    }
{----------------------------------------------------------------------}

{ Thu tuc tao ra mot Frame co toa do goc tren ben trai
  la (x1,y1) va goc duoi ben phai la (x2,y2). Tham so
  Lane quyet dinh kieu khung, voi Lane=1 ta co khung
  don, Lane=2 ta co khung kep, Lane=3 ta co khung phia
  tren kep, Lane=4 ta co khung phia hai ben kep }
PROCEDURE Frame(x1,y1,x2,y2,Lane : Byte);
CONST
  Bound :ARRAY[1..4] OF STRING[6]=
  (#218#196#191#179#217#192,#201#205#187#186#188#200,
  #213#205#184#179#190#212,#214#196#183#186#189#211);
VAR
  Border :STRING[6];
  K      :Integer;

  BEGIN
    Lane:=((Lane-1) MOD 4)+1;
    { De phong truong hop day bang }
    Border:=Bound[Lane];
    WriteXY(x1,y1,Border[1]);
    FOR k:=x1+1 TO x2-1 DO Write(Border[2]);
    Write(Border[3]);
    WriteXY(x1,y2,Border[6]);
    FOR k:=x1+1 TO x2-1 DO Write(Border[2]);
    Write(Border[5]);
    FOR k:=y1+1 TO y2-1 DO
      BEGIN
        WriteXY(x1,k,Border[4]);
        WriteXY(x2,k,Border[4])
      END
  END;

{ Thu tuc tao ra mot hop co mau nen va co ke khung.
  Tham so Color la mau nen cua hop, Skeleton la mau
  cua khung, Lane la kieu khung (Duoc giai thich o Frame) }
PROCEDURE Box(x1,y1,x2,y2,Color,Skeleton,Lane :Byte);
VAR
  BkColor,TxColor :Byte;
  BEGIN
    TxColor:=TextAttr MOD 16; { Lay lai mau chu hien tai }
    BkColor:=TextAttr DIV 16; { Lay lai mau nen hien tai }
    SetTextColor(Color,Skeleton);
    Frame(x1,y1,x2,y2,Lane);
    Window(x1+1,y1+1,x2-1,y2-1);
    ClrScr;
    Window(1,1,80,25);
    SetTextColor(BkColor,TxColor)
  END;

{ Ve hinh chu nhat co bong canh }
PROCEDURE BoxShadow(x1,y1,x2,y2,BkColor,BorderColor,Lane : Byte);
VAR OldTx,OldBk : Byte;
    BEGIN
      OldBk:=GetBkColor;
      OldTx:=GetTextColor;
      Shape(x1+2,y1+1,x2+2,y2+1);
      Window(x1,y1,x2,y2);
      SetTextColor(BkColor,BorderColor);
      Clrscr;
      Window(1,1,80,25);
      Box(x1,y1,x2,y2,BkColor,BorderColor,Lane);
      SetTextColor(OldBk,OldTx)
    END;

{----------------------------------------------------------------------}
{                CAC MODULE XU LY KICH THUOC CON TRO                   }
{----------------------------------------------------------------------}

{ Thiet lap kich thuoc cua con tro }
PROCEDURE SetCursor(Top,Bot : ShortInt);ASSEMBLER;
  ASM
    PUSH AX
    PUSH CX
    MOV  AH, $01
    MOV  CH, Top
    MOV  CL, Bot
    INT  $10
    POP  CX
    POP  AX
  END;

{----------------------------------------------------------------------}
{      CAC HAM THIET KE HE THONG MENU CUA CHUONG TRINH CHINH           }
{----------------------------------------------------------------------}
{ Tim chieu dai lon nhat cua cac option }
FUNCTION MaxLen( A : Items; Size : Byte) : Byte;
VAR Temp, j : Byte;
  BEGIN
    Temp := Length(A[1]);
    FOR j := 2 TO Size DO
      IF Length(A[j]) > Temp THEN
        Temp := Length(A[j]);
    MaxLen := Temp
  END;

{ Tim ky tu dau tin trong danh sach }
FUNCTION LookFor( A : Items; Ch : Char; Size : Byte ) : Byte;
VAR j : Byte;
  BEGIN
    j := 0;
    REPEAT
      Inc(j);
    UNTIL ( Upcase(Ch) = Upcase(A[j,1]) ) OR ( j = Size );
    IF Upcase(Ch) = Upcase(A[j,1]) THEN LookFor := j
    ELSE LookFor := 0
  END;

{ He thong Menu bar ngang }
FUNCTION MenuBar(x, y, Tx, Bk, ITx, IBk, BackC, TextC, F, Size : Byte;
                 A : Items) : Byte;
VAR
  k, Len, Choose : Byte;
  Ch             : Char;
  OldTx, OldBk   : Byte;

  { Dua cac option ra man hinh }
  PROCEDURE WriteItem( k : Byte; Active : Boolean);
    BEGIN
      IF Active THEN SetTextColor(IBk,ITx)
      ELSE SetTextColor(BackC,TextC);
      WriteXY( (k - 1) * (Len + 3) + 3 + x, y + 1, Replicate(' ',Len) );
      WriteXY( (k - 1) * (Len + 3) + 3 + x, y + 1, A[k] );
      TextColor( F );
      WriteXY( (k - 1) * (Len + 3) + 3 + x, y + 1, A[k,1] )
    END;

    { Ke khung cho Menu Bar }
    PROCEDURE PrintTable;
    VAR j : Byte;
      BEGIN
        SetCursor($20, $20);
        BoxShadow(x, y, (Len + 3) * Size + x + 3, y + 2, Bk, Tx, 1);
        FOR j := 2 TO Size DO WriteItem(j, FALSE);
        k := 1;
        WriteItem(k, TRUE)
      END;

  BEGIN { Menu Bar }
    OldTx := GetTextColor;
    OldBK := GetBkColor;
    Len := MaxLen(A, Size);
    PrintTable;
    Choose := 0;
    REPEAT
      Ch := Readkey;
      WriteItem(k, FALSE);
      CASE Ch OF
        #0 : BEGIN
               Ch := Readkey;
               CASE Ch OF
                 LEFT : IF k > 1 THEN Dec(k) ELSE k:=Size;
                 RIGHT : IF k < Size THEN Inc(k) ELSE k:=1;
                 HOME : k := 1;
                 ENDKEY : k := Size
               END
             END;
        ENTER : Choose := k;
        'A'..'Z', 'a'..'z' : Choose := LookFor(A, Ch, Size)
      END;
      WriteItem(k, TRUE)
    UNTIL Choose > 0;
    SetTextColor(OldBk, OldTx);
    MenuBar := Choose
  END;

{ He thong Menu PopUp }
FUNCTION MenuPop(x, y, Tx, Bk, ITx, IBk, F, Size : Byte; A : Items) : Byte;
VAR
  k, Len, Choose : Byte;
  Ch             : Char;
  OldTx, OldBk   : Byte;

  { Dua cac Option ra man hinh }
  PROCEDURE WriteItem( k : Byte; Active : Boolean);
    BEGIN
      IF Active THEN SetTextColor(IBk,ITx)
      ELSE SetTextColor(Bk,Tx);
      WriteXY(x + 1, y + k, Replicate(' ',Len+2) );
      WriteXY(x + 2, y + k, A[k] );
      TextColor( F );
      WriteXY(x + 2, y + k, A[k][1] )
    END;

    { Ke khung cho Menu PopUp }
    PROCEDURE PrintTable;
    VAR j : Byte;
      BEGIN
        SetCursor($20, $20);
        BoxShadow(x, y, Len + x + 3, y + Size + 1, Bk, Tx, 1);
        FOR j := 2 TO Size DO WriteItem(j, FALSE);
        k := 1;
        WriteItem(k, TRUE)
      END;

  BEGIN { Menu PopUp }
    OldTx := GetTextColor;
    OldBK := GetBkColor;
    Len := MaxLen(A, Size);
    PrintTable;
    Choose := 0;
    REPEAT
      Ch := Readkey;
      CASE Ch OF
        #0 : BEGIN
               Ch := Readkey;
               WriteItem(k, FALSE);
               CASE Ch OF
                 UP : IF k > 1 THEN Dec(k) ELSE k:=Size;
                 DOWN : IF k < Size THEN Inc(k) ELSE k:=1;
                 HOME : k := 1;
                 ENDKEY : k := Size
               END
             END;
        ENTER : Choose := k;
        'A'..'Z', 'a'..'z' : Choose := LookFor(A, Ch, Size)
      END;
      WriteItem(k, TRUE)
    UNTIL (Choose > 0) OR (Ch = ESC);
    SetTextColor(OldBk, OldTx);
    MenuPop := Choose
  END;

{----------------------------------------------------------------------}
{        CAC MODULE XU LY BAI HAT VA CAC KY TU TO DE TRANG TRI         }
{----------------------------------------------------------------------}

{ Lua chon cac ky tu co mau chu va mau nen de the hien ten bai hat }
PROCEDURE ShowTitleSongs(Strs : ArrStr; NumOption, x1, y1, x2, y2, BkColor0,
                         BkColor1, TxColor0, TxColor1, BoxColor, Lane : Byte;
                         VAR ResultOption, ResultKey : Byte);
VAR
  i,j,Len,
  Change  : Byte;
  Ch      : Char;
  Blank   : Str31;
  BEGIN
    BoxShadow(x1,y1,x2,y2,BkColor0,BoxColor,Lane);
    SayXY(((x2-x1) DIV 2-9)+x1,y1,LightGray,Blue,'Danh Sªch B§i Hªt');
    SayXY(((x2-x1) DIV 2-15)+x1,y2,LightGray,Blue,
           '<ENTER> : Chän / <ESC> : Thoªt');
    Len:=MaxLength(Strs,NumOption);
    IF Len>((x2-x1) DIV 2)-2 THEN Len:=((x2-x1) DIV 2)-2;
    FOR i:=1 TO NumOption DO
      BEGIN
        FOR j:=Length(Strs[i])+1 TO Len DO Strs[i][j]:=#32;
        Strs[i][0]:=Chr(Len)
      END;
    Change:=0;
    ResultOption:=1;
    Blank:='';
    FOR i:=(x1+1) TO ((x2+x1) DIV 2) DO Blank:=Blank+#32;
    REPEAT
      FOR i:=1 TO (y2-y1-1) DO
        BEGIN
          IF (2*(i+Change)-1)<=NumOption THEN
            IF (2*i-1)<>ResultOption THEN
              WriteColorXY(x1+1,y1+i,BkColor0,TxColor0,Strs[2*(Change+i)-1])
            ELSE WriteColorXY(x1+1,y1+i,BkColor1,TxColor1,Strs[2*(Change+i)-1]);
          IF (2*(i+Change))<=NumOption THEN
            IF (2*i)<>ResultOption THEN
              WriteColorXY(((x2-x1) DIV 2)+2+x1,y1+i,BkColor0,TxColor0,
                    Strs[2*(Change+i)])
            ELSE WriteColorXY(((x2-x1) DIV 2)+2+x1,y1+i,BkColor1,TxColor1,
                       Strs[2*(Change+i)])
        END;
      Ch:=Readkey;
      IF Ord(Ch)=0 THEN
        BEGIN
          Ch:=Readkey;
          CASE Ch OF
            Up   : IF ResultOption > 2 THEN ResultOption:=ResultOption-2
                   ELSE IF Change > 0 THEN Dec(Change);
            Down : IF (ResultOption MOD 2=0) THEN
                     BEGIN
                       IF (NumOption-(2*Change+ResultOption))>=2 THEN
                         IF (ResultOption<2*(y2-y1-1)) THEN
                           ResultOption:=ResultOption+2
                         ELSE inc(Change)
                     END
                   ELSE
                     IF (NumOption-(2*Change+ResultOption))>=2 THEN
                       IF (ResultOption=2*(y2-y1-1)-1) THEN
                          BEGIN
                             Inc(Change);
                             WriteColorXY(((x1+x2) DIV 2),y2-1,BkColor0,
                                   TxColor0,Blank)
                          END
                       ELSE
                         ResultOption:=ResultOption+2;
            Left,
            Right: IF (ResultOption MOD 2)=0 THEN
                      ResultOption:=ResultOption-1
                   ELSE
                     BEGIN
                       IF (ResultOption+2*Change)<NumOption THEN
                       ResultOption:=ResultOption+1;
                       IF ((ResultOption-1) div 2)+1=(y2-y1-1) THEN
                       WriteColorXY(((x1+x2) DIV 2),y2-1,BkColor0,TxColor0,Blank)
                     END;
          END { Of Case}
        END;
      UNTIL (Ch=Esc) OR (Ch=Enter);
      ResultKey:=Ord(Ch);
      ResultOption:=(ResultOption+2*Change)
    END;

{ Xac lap mau cua ky tu duoc chon }
PROCEDURE BackChar(x1, y1, x2, y2, ColorBackChar, ColorBox : Byte;
                   BackChar : Char);
VAR i,j,OldBk,OldTx : Byte;
  BEGIN
    OldBk:=GetBkColor;
    OldTx:=GetTextColor;
    SetTextColor(ColorBox,ColorBackChar);
    IF x1<1 THEN x1:=1;
    IF y1<1 THEN y1:=1;
    IF x2>80 THEN x2:=80;
    IF y2>25 THEN y2:=25;
    FOR i:=x1 TO x2 DO
      FOR j:=y1 TO y2 DO
        BEGIN
         Gotoxy(i,j);
         Write(BackChar)
        END;
    Window(1,1,80,1);
    Clrscr;
    Window(1,25,80,25);
    Clrscr;
    Window(1,1,80,25);
    SetTextColor(OldBk,OldTx)
  END;

{ Lam day mot vung man hinh bang ky tu ASCII }
PROCEDURE PutBigChar(x,y : Byte; Symbol : Char; CharPaint : Char);
VAR Line,Col    : 1..8;
    CurrentLine : Byte;
    Ch          : ArrChar;
    BEGIN
      Ch:=TableChar[Ord(Symbol)];
      FOR Line:=1 TO 8 DO
        BEGIN
          CurrentLine:=Ch[Line];
          FOR Col:=8 DOWNTO 1 DO
            BEGIN
              IF CurrentLine AND $01 <> 0 THEN
                WriteXY(x+col-1,y+line-1,CharPaint);
                CurrentLine:=CurrentLine SHR 1
            END
        END
    END;

{ Lay lai dia chi cua man hinh }
PROCEDURE SaveAllScreen(VAR AllMonitor : AllScreen);
VAR i,j : Byte;
    Count : Word;
  BEGIN
    Count:=0;
    FOR i:=1 TO 25 DO
      FOR j:=1 TO 80 DO
        BEGIN
          Inc(Count);
          AllMonitor[Count].Point:=MemW[SegmentVideo:Offset(j,i)]
        END
  END;

{ Thiet lap lai dia chi cua man hinh }
PROCEDURE RestoreAllScreen(AllMonitor : AllScreen);
VAR
  i,j   : Byte;
  Count : Word;
  BEGIN
    Count:=0;
    FOr i:=1 TO 25 DO
      FOR j:=1 to 80 DO
        BEGIN
          Inc(Count);
          MemW[SegmentVideo:Offset(j,i)]:=AllMonitor[Count].Point
        END
  END;

{ Lay lai vung man hinh }
PROCEDURE SavePartScreen(x1,y1,x2,y2 : Byte; VAR PartMonitor:PartScreen);
VAR
  i,j      : Byte;
  Monitor1,
  Monitor  : PartScreen;
  BEGIN
    FOR i:=y1 TO y2 DO
      FOR j:=x1 TO x2 DO
        BEGIN
          New(Monitor);
          Monitor^.Element.Point:=MemW[SegmentVideo:Offset(j,i)];
          Monitor^.Next:=NIL;
          IF (i=y1) AND (j=x1) THEN PartMonitor:=Monitor
          ELSE Monitor1^.Next:=Monitor;
          Monitor1:=Monitor
        END
  END;

{ Thiet lap mot vung man hinh }
PROCEDURE RestorePartScreen(x1,y1,x2,y2 : Byte; PartMonitor : PartScreen);
VAR
  i,j     : Byte;
  Monitor : PartScreen;
  BEGIN
    Monitor:=PartMonitor;
    FOR i:=y1 TO y2 DO
      FOR j:=x1 TO x2 DO
        BEGIN
          MemW[SegmentVideo:Offset(j,i)]:=Monitor^.Element.Point;
          Monitor:=Monitor^.Next
        END
  END;
{----------------------------------------------------------------------}
{         KET THUC CAC MODULE PHUC VU CHUONG TRINH KARAOKE.PAS         }
{----------------------------------------------------------------------}

{ Tao dong chu chay ngang man hinh o muc huong dan
  su dung chuong trinh va muc ban quyen }
PROCEDURE Advertising1;
CONST St : STRING = Chr(17)+Chr(17)+Chr(17)+Chr(17)+
' Ch¥¤ng tr×nh cã sù hîp tªc cña Trung t¡m ŸiÖn toªn v§ Ngo«i ng÷ CADASA ';
  BEGIN
    REPEAT
      PutString(St,3,24,1,100)
    UNTIL KeyPressed
  END;

{ Tao dong chu chay trong phan tom tat thong tin }
PROCEDURE Advertising2;
CONST Mess : STRING = Chr(17)+Chr(17)+
' T£i xin ch¡n th§nh cªm ¤n th¶y T£n Th½t Héi, b«n Phïng ThÞ Tr¡m Anh cïng'
+' t¾p thÓ líp 12A1 chuy¢n tin tr¥êng Long Khªnh ¦© gióp t£i ho§n th§nh '+
'ch¥¤ng tr×nh n§y '+Chr(16)+Chr(16);
  BEGIN
    TextbackGround(Blue);
    REPEAT
      SwapChar(Mess)
    UNTIL KeyPressed;
    GotoXY(1,1)
  END;

{ Ket thuc chuong trinh }
PROCEDURE EndProgram;
  BEGIN
    SetBorderColor($0); { Thiet lap mau bien mac nhien  }
    SetCursor($06,$07); { Thiet lap con tro mac nhien   }
    TextMode(LastMode); { Thiet lap lai che do man hinh }
    CloseMouse;         { Dong driver thiet bi chuot    }
    {LoadFont('0');      { Giai phong bo font tieng viet }
    ClrScr;             { Xoa man hinh ket qua          }
    Halt(1);            { Thoat khoi chuong trinh chinh }
  END;

{ Thu tuc kiem tra Password de bao ve ban quyen }
PROCEDURE CheckPassword;
VAR
  PassW1,PassW2 : STRING[15];
  Key           : Char;
  i, j          : Byte;
  Flag          : Boolean;
  BEGIN
    DoBlinking(TRUE);
    PassW1:='NGUYEN NGOC VAN';
    SetTextColor(Blue,Yellow);
    ClrScr;
    LoadFont('1');
    SetBorderColor(47);
    FillCharFrame(1,1,80,25,Blue,#178);
    { Lap thuoc tinh mau cho ky tu nen }
    SetTextColor($0,$0);
    PutBigChar(12,4,'K',#219);
    PutBigChar(22,4,'A',#219);
    PutBigChar(30,4,'R',#219);
    PutBigChar(40,4,'A',#219);
    PutBigChar(49,4,'O',#219);
    PutBigChar(58,4,'K',#219);
    PutBigChar(67,4,'E',#219);
    { Lap thuoc tinh mau cho ky tu chu }
    SetTextColor($0,$B);
    PutBigChar(10,3,'K',#219);
    PutBigChar(20,3,'A',#219);
    PutBigChar(28,3,'R',#219);
    PutBigChar(38,3,'A',#219);
    PutBigChar(47,3,'O',#219);
    PutBigChar(56,3,'K',#219);
    PutBigChar(65,3,'E',#219);
    { Tra lai thuoc tinh mac nhien }
    SetTextColor($1,$F);
    SetBorderColor(47);
    SetCursor($20,$20);
    BoxShadow(20,14,60,18,LightGray,Yellow,2);
    SayXY(35,14,Red,White,'M¾t kh·u');
    SayXY(39,16,Blue,White,Replicate(#32,13));
    SayXY(22,16,LightGray,Black,'Nh¾p m¾t kh·u :');
    SaveAllScreen(Monitor);
    REPEAT
      LoadFont('1');
      RestoreAllScreen(Monitor);
      i:=1;
      SetTextColor(Blue,White);
        REPEAT
          Key := Readkey;
          IF (i < 16) AND (Key <> #13) AND (Key <> #8) THEN
            BEGIN
              PassW2[i] := Key;
              WriteXY(38+i,16,'*');
              Inc(i)
            END
          ELSE
            IF (Key = #8) AND (i > 1) THEN
              BEGIN
                WriteXY(38+i-1,16,' ');
                Dec(i)
              END
            ELSE
              IF Key <> #13 THEN
                BEGIN
                  Sound(500);
                  Delay(100);
                  Nosound
                END
        UNTIL Key = #13;
        Flag := TRUE;
        FOR j:=1 TO 15 DO
          IF PassW1[j] <> PassW2[j] THEN Flag := FALSE;
        IF Flag THEN
          BEGIN
            BoxShadow(20,14,60,19,LightGray,Yellow,2);
            SayXY(36,14,Red,White,'KÕt Qu¨');
            SayXY(32,16,LightGray,Black+Blink,'Xin Chóc Mõng B«n!');
            SayXY(22,17,LightGray,Red,'Nh½n mét phÝm b½t kú ¦Ó tiÕp tôc...');
            SetCurSor($20,$20);
            Key:=ReadKey;
            IF Key=#27 THEN EndProgram;
          END
        ELSE
          BEGIN
            BoxShadow(20,14,60,19,Magenta,White,2);
            SayXY(36,14,Red,White,'KÕt Qu¨');
            SayXY(31,16,Magenta,Yellow+Blink,'M¾t kh·u bÞ sai!');
            SayXY(23,17,Magenta,Yellow,
            '<ENTER> : TiÕp tôc / <ESC> : Thoªt');
            SetCurSor($20,$20);
            Key:=ReadKey;
            IF Key=#27 THEN EndProgram;
          END
    UNTIL Flag
  END; { Ket thuc thu tuc nhap Password }

{------------ Nhay sang trang ke -----------------}
PROCEDURE NextPage;
VAR I : Integer;
  BEGIN
    GotoXY(1,1);
    FOR I:=1 TO 25 DO
      BEGIN
        Delline;
        Delay(10)
      END
  END;

{----------------------------------------------------------------------}
{              CAC MODULE XU LY CHUONG TRINH KARAOKE                   }
{----------------------------------------------------------------------}

{ Ve cac not nhac xung quanh man hinh }
PROCEDURE RoundScreen;
VAR j : Byte;
  BEGIN
    FOR j:=24 DOWNTO 1 DO
      WriteColorXY(1,j,9,((j-1) MOD 14)+2,#3);
    FOR j:=1 TO 80 DO
      WriteColorXY(j,1,9,((j-1) MOD 14)+2,#3);
    FOR j:=1 TO 24 DO
      WriteColorXY(80,j,9,((j-1) MOD 14)+2,#3);
  END;

{ Trinh bay cac ky tu tao thanh chu KARAOKE }
PROCEDURE DisplayScreen;
VAR RandColor : Byte;
  BEGIN
    Randomize;
    RandColor:=Random(5)+10;
    IF RandColor = 13 THEN RandColor:=15;
    BackChar(1,1,80,25,$D,$0,#219); { Thiet lap mau nen cho cac ky tu }
    RoundScreen; { Ve cac not nhac xung quanh man hinh }
    SetAttrib(1,25,80,25,$1,$1); { Thanh tieu de o day man hinh }
    SetTextColor($0,$8);
    PutBigChar(12,4,'K',#219);
    PutBigChar(22,4,'A',#219);
    PutBigChar(30,4,'R',#219);
    PutBigChar(40,4,'A',#219);
    PutBigChar(49,4,'O',#219);
    PutBigChar(58,4,'K',#219);
    PutBigChar(67,4,'E',#219);
    { Lap thuoc tinh mau cho ky tu chu }
    SetTextColor($0,RandColor);
    PutBigChar(10,3,'K',#219);
    PutBigChar(20,3,'A',#219);
    PutBigChar(28,3,'R',#219);
    PutBigChar(38,3,'A',#219);
    PutBigChar(47,3,'O',#219);
    PutBigChar(56,3,'K',#219);
    PutBigChar(65,3,'E',#219);
    SaveAllScreen(Monitor);
    SetTextColor($1,$15)
    { Tra lai thuoc tinh mau mac nhien }
  END;

{ Doc cac ten FILE nhac vao danh sach bai hat }
PROCEDURE InputFilesName(Path :Str31; VAR Key : Byte) ;
VAR
  NumSong,NameSong : ArrStr;
  Opt,NumS,
  i,j              : Byte;
  F                : Text;
  Title            : Str31;
  BEGIN
    Stop:=ListSongs(Path+'*.NOT',NumSong,NumS);
    IF NOT Stop THEN
      BEGIN
        SetTextColor(Blue,Yellow);
        ClrScr;
        WriteXY(15,11,'Chó ý : Kh£ng th½y cªc t¾p tin *.not!. B«n h©y v§o');
        WriteXY(15,12,'th¥ môc cã chøa cªc t¾p tin nãi tr¢n míi ch«y ¦¥îc');
        WriteXY(15,13,'ch¥¤ng tr×nh. H©y gâ phÝm <ENTER> ¦Ó thoªt.');
        Readln;
        EndProgram;
      END;
    FOR i:=1 TO NumS DO
      BEGIN
        NumSong[i] := Path+NumSong[i];
        Assign(f,NumSong[i]);
        Reset(f);
        Readln(f,Title);
        Close(f);
        j:=1;
        WHILE ((Title[j]=' ') OR (Title[j]='0'))
        AND (j<Length(Title)+1) DO Inc(j);
	NameSong[i]:='  '+Copy(Title,j,Length(Title)-j+1)+'  ';
      END;
    IF Stop THEN
      BEGIN
        ShowTitleSongs(NameSong,NumS,10,12,70,22,$7,$4,$0,$F,$A,2,Opt,Key)
      END;
    Fn:=NumSong[Opt]
  END;

{ Doc du lieu cua tung FILE vao bo nho }
PROCEDURE InputDataRam;
VAR
  F           : Text;
  Hight,Width : Word;
  St          : Str40;
  P           : Byte;
  BEGIN
    Assign(f,fn);
    Reset(f);
    k:=0;
    WHILE NOT Eof(f) DO
      BEGIN
        Inc(k);
        {$I-}
        Readln(f,Hight,Width,St);
        {$I+}
        IF (DosError=18) OR (DosError=0) THEN
        { Khong xay ra loi, cong viec doc thanh cong }
          BEGIN
            WHILE (Length(St)>0) AND (St[Length(St)]=' ') DO
              Delete(St,Length(St),1);
            WHILE (Length(St)>0) AND (St[1]=' ') DO Delete(St,1,1);
            IF ((St='') OR (St='~')) THEN
              IF (Hight<>0) THEN St:=#3+St
              ELSE St:=#4+St
            ELSE St:=' '+St;
            WITH Songs[k] DO
              BEGIN
                HightL:=Hight;
                WidthL:=Width;
                CharL:=St
              END
          END
        ELSE Halt(1)
        { Xay ra loi, cong viec doc khong thanh
          cong dung lap tuc chuong trinh }
      END;
    Close(f)
  END;

{ Dien cac not nhac o dong day moi ban nhac
  moi not nhac nay truong trungcho den nhay }
PROCEDURE Light(NumChar : Byte);
VAR i : Byte;
  BEGIN
    IF NumChar > 0 THEN
      FOR i:=1 TO NumChar DO
         BEGIN
            IF i <= 10 THEN
               BEGIN
                  TextColor(BLACK);
                  Write(#14);
               END
            ELSE
               IF i <= 20 THEN
                  BEGIN
                     TextColor(BLUE);
                     Write(#14);
                  END
               ELSE
                  IF i <= 30 THEN
                     BEGIN
                        TextColor(WHITE);
                        Write(#14);
                     END
                  ELSE
                     IF i <= 40 THEN
                        BEGIN
                           TextColor(LIGHTRED);
                           Write(#14);
                        END
                     ELSE
                        BEGIN
                           TextColor(LIGHTCYAN);
                           Write(#14);
                        END;
         END;
  END;

{ Thiet lap hieu ung cua den theo cao do va truong do }
PROCEDURE Music(x,y,BkColor,TxColor : Byte; HeightVoid: Word);
VAR OldBk,OldTx : Byte;
  BEGIN
    Sound(HeightVoid);
    OldBk:=GetBkColor;
    OldTx:=GetTextColor;
    SetTextColor(BkColor,TxColor);
    GotoXY(x,y);
    CASE HeightVoid OF
      80..180    : Light(1);
      181..201   : Light(2);
      202..213   : Light(3);
      214..226   : Light(4);
      227..240   : Light(5);
      241..254   : Light(6);
      255..269   : Light(7);
      270..285   : Light(8);
      286..301   : Light(9);
      302..310   : Light(10);
      311..338   : Light(11);
      339..359   : Light(12);
      360..378   : Light(13);
      379..403   : Light(14);
      404..426   : Light(15);
      427..453   : Light(16);
      454..479   : Light(17);
      480..507   : Light(18);
      508..537   : Light(19);
      538..569   : Light(20);
      570..603   : Light(21);
      604..640   : Light(22);
      641..674   : Light(23);
      675..714   : Light(24);
      715..754   : Light(25);
      755..804   : Light(26);
      805..854   : Light(27);
      855..904   : Light(28);
      905..964   : Light(29);
      965..1024  : Light(30);
      1025..1074 : Light(31);
      1075..1134 : Light(32);
      1135..1204 : Light(33);
      1205..1274 : Light(34);
      1275..1354 : Light(35);
      1355..1434 : Light(36);
      1435..1524 : Light(37);
      1525..1604 : Light(38);
      1605..1709 : Light(39);
      1710..1809 : Light(40);
      1810..1914 : Light(41);
      1915..2024 : Light(42);
      2025..2154 : Light(43);
      2155..2184 : Light(44);
      2195..2414 : Light(45);
      2415..2564 : Light(46);
      2565..2724 : Light(47);
      2725..2874 : Light(48);
      2875..3024 : Light(49);
      3025..3204 : Light(50);
      3205..3420 : Light(51);
      3421..3620 : Light(52);
      3621..3840 : Light(53);
      3841       : Light(54);
    ELSE Light(0)
    END;
    SetTextColor(OldBk,OldTx);
    { Tra lai mau chu va nen mac nhien }
  END;

{ Xoay cac ky tu duoc lay ra bang ASCII va tao truong do loi hat }
PROCEDURE RotateChars(y, BkColor,TxColor : Byte; Times, Ordre : Word;
                      VAR St : STRING);
  VAR
    i,j   : Byte;
    Ch    : Char;
    Tempo : STRING;
  BEGIN
    IF Length(St) < 78 THEN
      FOR i:=Length(St) TO 78 DO St:= '.'+St;
    TextAttr:=(BkColor SHL 4)+TxColor;
    { Thiet lap mau chu va mau nen }
    FOR i:=1 TO Ordre DO
      BEGIN
        Tempo:=St;
        Tempo[0]:=Chr(78);
        GotoXY(2,y);
        Write(Tempo);
        Delay(Times);
        Ch:=St[1];
        FOR j:=1 TO Length(St)-1 DO St[j]:=St[j+1];
        St[Length(St)]:=Ch
      END
  END;

{ Viet cac ky tu trong loi ra man hinh }
PROCEDURE WriteWords(St : Str31; VAR x,y : Byte);
  BEGIN
    IF (X+Length(St))>X2 THEN
      BEGIN
        IF y<y2p THEN Inc(y)
        ELSE y:=y2p;
        x:=x1
      END;
    GotoXY(x,y);
    IF St[Length(St)]<>'~' THEN Write(St)
    ELSE Write(Copy(St,1,Length(St)-1));
    x:=WhereX
  END;

{ Bat dau choi KARAOKE }
PROCEDURE PlayMusic(VAR Ch : Char) ;
  VAR
    I       : Word;
    X,Y,T   : Byte;
    St      : STRING;
    Monitor : PartScreen;

  { Load tung tua de cua ban nhac }
  PROCEDURE Title;
    BEGIN
      i:=1;
      IF (Songs[1].WidthL=0) THEN
        BEGIN
          WriteColorXY(39-(Length(Songs[1].CharL)) DIV 2,y1p-2,Red,White,
          Songs[1].CharL+' ');
          i:=2
        END;
      IF (Songs[2].WidthL=0) THEN
        BEGIN
          WriteColorXY(x2-(Length(Songs[2].CharL))-3,y1p-1,Cyan,Yellow,
          Songs[2].CharL);
          i:=3
        END
    END;

  { Viet ra loi va to mau cac ky tu }
  PROCEDURE WriteChar(i : Word; TxColor : Byte);
    BEGIN
      y:=y1p;
      IF i<5 THEN x:=x1+5 ELSE x:=x1;
      WHILE (i<=k) AND (y<y2p) DO
        BEGIN
          IF Pos(#3,Songs[i].CharL)<>0 THEN TextAttr:=$3E
          ELSE
            IF Pos(#4,Songs[i].CharL)<>0 THEN TextAttr:=$3C
            ELSE TextAttr:=$30+TxColor;
          WriteWords(Songs[i].CharL,x,y);
          Inc(i)
        END;
      IF (i<=k) AND (y=y2p) THEN
        WHILE (x+Length(Songs[i].CharL))< x2 DO
          BEGIN
            IF Pos(#3,Songs[i].CharL)<>0 THEN TextAttr:=$3E
            ELSE
              IF Pos(#4,Songs[i].CharL)<>0 THEN TextAttr:=$3C
              ELSE TextAttr:=$30+TxColor;
            WriteWords(Songs[i].CharL,x,y);
            Inc(i)
          END
    END;

  { Tao kich thuoc khung phu hop voi chieu dai cua ban nhac }
  PROCEDURE BoxSize;
    BEGIN
      x:=X1+5;
      i:=1;
      y2p:=y1p;
      WHILE (i<k) AND (y2p<22) DO
        BEGIN
          IF (x+Length(Songs[i].CharL)+1)<=x2 THEN
            x:=x+Length(Songs[i].CharL)+1
          ELSE
            BEGIN
              x:=x1;
              Inc(y2p)
            END;
          Inc(i)
        END;
      IF y2p>21 THEN y2p:=21
    END;

  { Cac not nhac chop tat xung quanh khung man hinh }
  PROCEDURE LightFlash(i : Word);
  VAR j : Word;
    BEGIN

      { Khun phia be trai }
      FOR j:=24 DOWNTO 1 DO
        BEGIN
          SetAttrib(1,j,1,j,((i-1) MOD 14) + 2,1);
          Inc(i)
        END;

      { Khung phai ben tren }
      FOR j:=1 TO 80 DO
        BEGIN
          SetAttrib(j,1,j,1,((i-1) MOD 14) + 2,1);
          Inc(i)
        END;

      { Khung phia ben phai }
      FOR j:=1 TO 24 DO
        BEGIN
          SetAttrib(80,j,80,j,((i-1) MOD 14) + 2,1);
          Inc(i)
        END
    END;

  { Thanh dieu chinh do nhanh cham cua ban nhac }
  PROCEDURE BarVolume(n : Byte);
  CONST x = 2; y = 2;
  VAR i : Byte;
    BEGIN
      TextAttr:=$0D;
      WriteXY(x,y,'ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ');
      TextAttr:=$0F;
      FOR i:=1 TO n DO WriteXY(x-1+i,y,#219);
      IF n>0 THEN WriteColorXY(x-1+i,y,$0,$0,#219);
      TextAttr:=$DB;
      WriteXY(x,y+1,'Fast');
      WriteXY(x+7,y+1,'Middle');
      WriteXY(x+17,y+1,'Slow');
    END;

  { Thuc hien viec doc cac cao do, truong do va to loi bai hat }
  PROCEDURE RunChars(TxColor: Byte);
  CONST x0=2;
        y0=2;
    BEGIN
      Ch:=' ';
      TextAttr:=$0F;
      WriteXY(x0+18,y0,#17);
      BarVolume(WidthLen);
      WHILE (i<=k) AND (Ch<>ESC) AND ((y<y2p) OR
      (x<=x2-length(Songs[i].CharL))) DO
      WITH Songs[i] DO
        BEGIN
          Music(x1,y2p+1,$3,$4,HightL); { Caodo+Den }
          SetTextColor($3,TxColor);
          WriteWords(CharL,x,y);
          t:=(WidthL*WidthLen) DIV Tg;
          IF ((WidthL*WidthLen) MOD Tg) > 0 THEN
          Delay((WidthL*WidthLen) MOD Tg);
          LightFlash(i);
          RotateChars(25,1,15,Tg,T,Adver);
          IF (CharL[Length(CharL)]<>'~') THEN NoSound;
	  RestorePartScreen(2,y2p+1,78,y2p+1,Monitor);
          IF Keypressed THEN
            BEGIN
              Ch:=Readkey;
              IF Ch=#0 THEN
                BEGIN
                  Ch:=Readkey;
                  { Dinh dang thanh truoc de tang hoac giam
                    do nhanh cham khi hat }
                  CASE Ch OF
                    Up,Right : IF WidthLen < 18 THEN
                          BEGIN
                            Inc(WidthLen);
                            BarVolume(WidthLen)
                          END;
                    Down,Left : IF WidthLen > 1 THEN
                          BEGIN
                            Dec(WidthLen);
                            BarVolume(WidthLen)
                          END
                  END { Case }
                END { Ch=#0 }
            END; { Keypressed }
          Inc(i)
        END;  { WITH Songs[i] }
      Nosound { Ngung phat am thanh }
    END; { PROCEDURE Run }

  { Than cua PROCEDURE PlayMusic }
  BEGIN
    BoxSize;
    BoxShadow(x1-1,y1p-2,x2+1,y2p+1,Cyan,Yellow,2);
    Title;
    WriteChar(i,$F);
    x:=x1+5;
    y:=y1p;
    IF Songs[1].WidthL=0 THEN
      IF Songs[2].WidthL=0 THEN i:=3 ELSE i:=2
      ELSE i:=2;
    SavePartScreen(2,y2p+1,78,y2p+1,Monitor);
    RunChars(Black);
    WHILE (i<k) AND (Ch<>Esc) DO
      BEGIN
        Window(x1,y1p,x2,y2p);
        TextBackGround($3);
        ClrScr;
        Window(1,1,80,25);
        WriteChar(i,$F);
        x:=x1;
        y:=y1p;
        RunChars(Black)
      END
  END;

{ Thong bao may dang nap du lieu vao bo nho }
PROCEDURE Waitting;
  BEGIN
    BoxShadow(22,17,59,19,Red,Yellow,2);
    GotoXY(24,18);
    SetTextColor(Red,LightCyan+Blink);
    Writeln('Ÿang n«p d÷ liÖu, ¦îi chót xÝu ...');
    Delay(2000);
    Sound(2000);
    Delay(200);
    NoSound;
    Delay(120)
  END;

PROCEDURE BoxRunChar(x1,y1,x2,y2,EdgeC,TextC : Byte; Str : Str40);
VAR i,j : Byte;
  BEGIN
    SetCursor($20,$20);
    TextColor(EdgeC);
    Frame(x1,y1,x2,y2,2);
    TextColor(TextC);
    WriteXY(x1+1,y1+1,Str);
    WHILE NOT KeyPressed DO
      BEGIN
        FOR i:=x1+2 TO ((x2-x1)-Length(Str))+x1 DO
          BEGIN
            IF KeyPressed THEN EXIT;
            WriteXY(i,y1+1,Str);
            Delay(100);
            ClearPartScreen(x1+1,y1+1,x2-1,y2-1,Blue)
          END;
        FOR j:=i-1 DOWNTO x1+1 DO
          BEGIN
            IF KeyPressed THEN EXIT;
            WriteXY(j,y1+1,Str);
            Delay(100);
            ClearPartScreen(x1+1,y1+1,x2-1,y2-1,Blue)
          END
      END
  END;

PROCEDURE Introduction;
CONST Mess = 'Giíi thiÖu ch¥¤ng tr×nh KARAOKE';
  BEGIN
    SetCursor($20,$20);
    SetTextColor(Blue,Yellow);
    SetBorderColor(47);
    Clrscr;
    Frame(2,1,79,25,2);
    WriteXY(12,7,
    'Í Sau ¦¡y t£i xin tù giíi thiÖu vÒ s¨n ph·m ph¶n mÒm kh£ng');
    WriteXY(10,8,
    'kÐm ph¶n ch½t l¥îng n§y . Ph¶n mÒm KARAOKE ¦¥îc x¡y dùng tr¢n');
    WriteXY(10,9,
    'tr×nh hªt nh«c m§ cªc b«n v¸n th¥êng hay hªt ë nh§ hoµc ë cªc');
    WriteXY(10,10,
    'quªn VIDEO nh¥ng xin th¥a víi cªc b«n ¦¡y chØ l§ ch¥¤ng tr×nh');
    WriteXY(10,11,
    'm£ pháng ë c½p th½p m§ th£i (?!).');
    WriteXY(12,14,
    'Í NÕu cªc b«n cã høng thó víi s¨n ph·m n§y xin cªc b«n h©y');
    WriteXY(10,15,
    'thö søc ¦Ó cã thÓ phªt triÓn nã h¤n n÷a. Cªc b«n cã dªm kh£ng');
    WriteXY(10,16,
    't£i th× tin r¬ng víi sù th£ng minh cña b«n th× viÖc n§y ch­ng');
    WriteXY(10,17,
    'l§ v½n ¦Ò m§ b«n c¶n quan t¡m.');
    WriteColorXY(24,20,Blue,LightCyan,'H©y gâ mét phÝm b½t kú ¦Ó thoªt ...');
    BoxRunChar(20,3,60,5,LightGreen,White,Mess);
  END;

PROCEDURE LoadFonts;
CONST Mess = 'H¥íng d¸n cªch n«p fonts tiÕng viÖt';
  BEGIN
    SetCursor($20,$20);
    SetTextColor(Blue,Yellow);
    SetBorderColor(46);
    Clrscr;
    Frame(2,1,79,25,2);
    WriteXY(12,9,
    'Í Sau ¦¡y t£i xin h¥íng d¸n cªc b«n cªch n«p t¾p tin  tiÕng');
    WriteXY(10,10,
    'viÖt . Ch¥¤ng tr×nh n§y tù ¦éng n«p t¾p tin gi¨i m© tiÕng viÖt');
    WriteXY(10,11,
    'NÕu v× mét lý do n§o ¦ã m§ khi b«n ch«y ch¥¤ng tr×nh m§ nã  ra');
    WriteXY(10,12,
    'to§n l§ tiÕng "Campuchia" kh£ng (!?). Xin b«n chí cã hèt ho¨ng');
    WriteXY(10,13,
    'm§ h©y b×nh tÜnh xem l«i trong th¥ môc hiÖn h§nh coi th¬ng cha');
    WriteXY(10,14,
    '"Th£ng dÞch vi¢n" xem cßn sèng hay ¦© chÕt (file vn.com) , nÕu');
    WriteXY(10,15,
    'th¬ng ch¨ cßn sèng th× l£i cæ h¯n dz¾y , cßn nÕu nh¥ th¬ng ch¨');
    WriteXY(10,16,
    'kh£ng cªnh m§ biÕt bay th× b«n chÞu khã "mß" tiÕng "Campuchia"');
    WriteXY(10,17,
    'v¾y. Ÿõng quª th½t väng li¢n hÖ víi t£i ¦Ó "m¥ín" th¬ng ch¨ vÒ');
    WriteColorXY(24,20,Blue,LightCyan,'H©y gâ mét phÝm b½t kú ¦Ó thoªt ...');
    BoxRunChar(16,3,64,5,LightGreen,White,Mess);
  END;

PROCEDURE Information;
CONST Mess = 'Cªc th£ng tin vÒ ch¥¤ng tr×nh KARAOKE';
  BEGIN
    SetCursor($20,$20);
    SetTextColor(Blue,Yellow);
    SetBorderColor(39);
    Clrscr;
    Frame(2,1,79,25,2);
    WriteXY(20,12,'Ch¥a c¾p nh¾t kÞp mong cªc b«n l¥îng thø cho .......');
    WriteColorXY(24,22,Blue,LightCyan,'H©y gâ mét phÝm b½t kú ¦Ó thoªt ...');
    BoxRunChar(16,3,64,5,LightGreen,White,Mess)
  END;

PROCEDURE Usage;
CONST Mess = 'H¥íng d¸n sö dông ch¥¤ng tr×nh KARAOKE';
  BEGIN
    SetCursor($20,$20);
    SetTextColor(Blue,Yellow);
    SetBorderColor(63);
    Clrscr;
    Frame(2,1,79,25,2);
    WriteXY(12,7,
    'Í Ch¥¤ng tr×nh n§y ¦¥îc thiÕt kÕ sao cho thÝch hîp víi mäi');
    WriteXY(10,8,
    'thÕ hÖ mªy, cã giao diÖn r½t ng¶n gòi v§ r½t dÔ sö dông. Ph¶n');
    WriteXY(10,9,
    '¦¶u cã ch¥¤ng tr×nh xªc nh¾n m¾t kh·u ¦Ó b¨o vÖ b¨n quyÒn cña');
    WriteXY(10,10,
    'cña ch¥¤ng tr×nh.ŸÓ ¦¨m b¨o tÝnh b¨o m¾t n¢n m¾t kh·u cña b«n');
    WriteXY(10,11,
    '¦ªnh v§o kh£ng hiÖn trùc tiÕp ra m§n h×nh m§ chØ hiÖn d½u "*"');
    WriteXY(12,13,
    'Í Xin l¥u ý : khi gâ m¾t kh·u b«n ph¨i gâ b¬ng ch÷ hoa th×');
    WriteXY(10,14,
    'mªy míi ch½p nh¾n. Sau ¦ã l§ danh sªch cªc b§i hªt tiÕng viÖt');
    WriteXY(10,15,
    'hiÖn ra b«n muèn chän b§i n§o chØ c¶n dêi vÖt sªng m§u ¦á ¦Õn');
    WriteXY(10,16,
    'v§ gâ <ENTER> ¦Ó xªc nh¾n. C£ng viÖc cña r½t ¦¤n gi¨n l§ ngåi');
    WriteXY(10,17,
    '... chê cho mªy n«p b§i hªt ¦ã v§o RAM nÕu n«p th§nh c£ng th×');
    WriteXY(10,18,
    'phªt ra tiÕng Beep cßn nÕu kh£ng n«p ¦¥îc th× coi nh¥ b«n gµp');
    WriteXY(10,19,
    'xui xÇo (!?). Trong lóc ¦ang hªt b«n cã thÓ dïng phÝm mòi t¢n');
    WriteXY(10,20,
    't ng ¦é nhanh ch¾m cña b§i hªt dïng <ESC> ¦Ó chän b§i hªt míi');
    WriteColorXY(24,23,Blue,LightCyan,'H©y gâ mét phÝm b½t kú ¦Ó thoªt ...');
    BoxRunChar(16,3,65,5,LightGreen,White,Mess);
  END;

PROCEDURE Topics;
CONST Mess = 'Tæ chøc ch¥¤ng tr×nh KARAOKE';
  BEGIN
    SetCursor($20,$20);
    SetTextColor(Blue,Yellow);
    SetBorderColor(43);
    Clrscr;
    Frame(2,1,79,25,2);
    WriteXY(20,12,'Ch¥a c¾p nh¾t kÞp mong cªc b«n l¥îng thø cho .......');
    WriteColorXY(24,22,Blue,LightCyan,'H©y gâ mét phÝm b½t kú ¦Ó thoªt ...');
    BoxRunChar(20,3,60,5,LightGreen,White,Mess)
  END;

PROCEDURE OverAll;
VAR i : Byte;
  BEGIN
    SetCursor($20,$20);
    SetBorderColor(0);
    SetTextColor(Blue,Yellow);
    SetBorderColor(62);
    ClrScr;
    Frame(2,1,79,25,1);
    WriteXY(20,6,'Tr¥êng Ÿ«i Häc Kü Thu¾t C£ng NghÖ');
    WriteXY(25,7,'C¤ së I  : 110 Cao Th¯ng Qu¾n 3');
    WriteXY(25,8,'C¤ së II : 144/24 ŸiÖn Bi¢n Phñ');
    WriteXY(36,9,'Qu¾n B×nh Th«nh');
    WriteXY(20,10,'Mäi chi tiÕt xin li¢n hÖ t«i ¦i« chØ :');
    WriteXY(25,11,'- Phone   : 9300061');
    WriteXY(25,12,'- Website : http://www.unitech.vnn.vn');
    WriteXY(25,13,'- Email   : nguyenvan@yahoo.com');
    WriteXY(20,14,'Hoµc li¢n hÖ trùc tiÕp víi tªc gi¨ t«i :');
    WriteXY(20,15,'57A Ph¥êng 1 Qu¾n 4 Cï Lao NguyÔn KiÖu. ');
    WriteXY(2,17,#195);
    WriteXY(78,17,#196#180);
    WriteXY(3,17,Replicate(#196,75));
    WriteXY(7,19,
    'Xin l¥u ý : Ch¥¤ng tr×nh l§ phi¢n b¨n ch«y thö nghiÖm. C½m sao chÐp');
    WriteXY(7,20,
    'mét ph¶n hay c¨ ch¥¤ng tr×nh d¥íi b½t kú h×nh thøc n§o. Mäi vi ph«m');
    WriteXY(7,21,
    'sÏ bÞ truy tè tr¥íc héi ¦ång cña nh§ tr¥êng. NÕu cªc b«n cã nhu c¶u');
    WriteXY(7,22,
    'vÒ ch¥¤ng tr×nh nguån xin vui lßng li¢n hÖ t«i ¦Þa chØ tr¢n.');
    SayXY(25,25,Blue,LightGreen,'H©y gâ mét phÝm b½t kú ¦Ó thoªt');
    SayXY(30,1,Blue,LightGreen,'B¨n quyÒn ch¥¤ng tr×nh');
    Advertising1;
    FOR i:=12 DOWNTO 0 DO
      BEGIN
        ClearPartScreen(3*i,i,80-3*i,26-i,Random(MaxColors)+1);
        Sound(212*i);
        Delay(120-i);
        Nosound
      END
  END;

PROCEDURE Development;
CONST Mess = 'H¥íng phªt triÓn ch¥¤ng tr×nh';
  BEGIN
    SetCursor($20,$20);
    SetTextColor(Blue,Yellow);
    SetBorderColor(60);
    Clrscr;
    Frame(2,1,79,25,2);
    WriteXY(12,8,
    'Í Còng xin l¥u ý víi cªc b«n r¬ng ch¥¤ng tr×nh n§y chØ l§');
    WriteXY(10,9,
    'phi¢n b¨n ch«y thö nghiÖm cho n¢n tªc gi¨ khuyÕn cªo v¸n cßn');
    WriteXY(10,10,
    'cã sai xãt. NÕu trong quª tr×nh sö dông ch¥¤ng tr×nh cã phªt');
    WriteXY(10,11,
    'sinh lçi, h©y bªo ngay víi tªc gi¨ ¦Ó kÞp thêi ¦iÒu chØnh m©');
    WriteXY(10,12,
    'nguån . NÕu cªc b«n cã høng thó muèn phªt triÓn ch¥¤ng tr×nh');
    WriteXY(10,13,
    'n§y h¤n n÷a th× h©y li¢n hÖ víi tªc gi¨ ¦Ó nh¾n ch¥¤ng tr×nh');
    WriteXY(10,14,
    'nguån. Tªc gi¨ r½t mong cªc b«n nhiÖt t×nh h¥ëng øng ý t¥ëng');
    WriteXY(10,15,
    'n§y mµc dï ch¥¤ng tr×nh n§y còng kh£ng cã g× gh¢ gím l¯m.');
    WriteXY(12,17,
    'Í T£i ¦¡y r½t mong ¦¥îc sù nhiÖt t×nh ñng hé , sù hîp tªc');
    WriteXY(10,18,
    'quý bªo cña cªc b«n ¦Ó ch¥¤ng tr×nh cña t£i ng§y  c§ng  ¦¥îc');
    WriteXY(10,19,
    'nhiÒu ng¥êi ¥a chuéng h¤n . T£i s®n s§ng tiÕp thu mäi ý kiÕn');
    WriteXY(10,20,
    'qóy bªo cña cªc b«n, v§ xin ch¡n th§nh cªm ¤n r½t nhiÒu.');
    WriteColorXY(24,22,Blue,LightCyan,'H©y gâ mét phÝm b½t kú ¦Ó thoªt ...');
    BoxRunChar(20,3,60,5,LightGreen,White,Mess)
  END;

PROCEDURE Summary;
VAR i : Byte;
  BEGIN
    SetCurSor($20,$20);
    SetBorderColor(52);
    FillCharFrame(1,1,80,25,LightGreen,#219);
    BoxShadow(13,4,70,21,Magenta,White,2);
    SayXY(32,4,Magenta,LightGreen,'Tãm t¯t th£ng tin');
    SayXY(28,21,Magenta,LightGreen,'Nh½n phÝm <ENTER> ¦Ó thoªt');
    SetTextColor(Magenta,White);
    WriteXY(16,6,'R½t cªm ¤n cªc b«n ¦© ñng hé v§ sö dông ch¥¤ng tr×nh');
    WriteXY(16,7,'cña t£i. Mong cªc b«n gãp ý vÒ néi dung  cña  ch¥¤ng');
    WriteXY(16,8,'tr×nh ¦Ó ch¥¤ng tr×nh ¦¥îc ho§n  thiÖn  h¤n. NÕu cªc');
    WriteXY(16,9,'b«n cã "mªu" m¢ nh«c xin h©y viÕt th¢m cªc File nh«c');
    WriteXY(16,10,'(*.not) bæ sung v§o th¥ môc chay ch¥¤ng tr×nh v§ b«n');
    WriteXY(16,11,'¦õng qu¢n gëi cho t£i mét b¨n nhÐ! xin cªm ¤n tr¥íc.');
    WriteXY(16,12,'HÑn gµp l«i cªc b«n ë mét ¦Ò t§i míi mÇ khªc.');
    Frame(26,14,58,20,1);
    WriteColorXY(33,15,Magenta,Yellow,'Ch¥¤ng Tr×nh KARAOKE');
    SetTextColor(Magenta,LightCyan);
    WriteXY(28,16,'Ng§y viÕt  : 25/09/2000');
    WriteXY(28,17,'Ho§n th§nh : 03/01/2001');
    WriteXY(28,18,'Ng£n ng÷   : C++, Assembler');
    WriteXY(28,19,'Ng¥êi viÕt : NguyÔn Ngäc V¡n');
    Advertising2;
    Readln;
    FOR i:=12 DOWNTO 0 DO
      BEGIN
        ClearPartScreen(3*i,i,80-3*i,26-i,Blue);
        Delay(100)
      END
  END;

{ Khoi tao chuong trinh KaraOke }
PROCEDURE InitializeKaraoke;
  BEGIN
    SetCursor($20,20);
    SetBorderColor(54);
    REPEAT
      DisplayScreen;
      RestoreAllScreen(Monitor);
      {InputFilesName(GetDisk+'TOPICS\KARAOKE\',Key);}
      InputFilesName('DATA\',Key);
      IF (Key <> 27) AND Stop THEN
        BEGIN
          RestoreAllScreen(Monitor);
          Waitting;
          FillCharFrame(8,3,75,22,$0D,#219);
          InputDataRam;
          REPEAT
            PlayMusic(Ch);
          UNTIL (Ch=ESC)
        END;
    UNTIL (Key = 27) OR NOT Stop
  END;

PROCEDURE StartProgram;
VAR
  Monitor : AllScreen;
  Choose  : Byte;
  Ch      : Char;
  BEGIN
  {  LoadFont('1');}
    SetBorderColor(55);
    SetTextColor(Blue,White);
    ClrScr;
    FillCharFrame(1,1,80,25,LightBlue,#178);
    { Lap thuoc tinh mau cho ky tu nen }
    SetTextColor($0,$0);
    PutBigChar(12,15,'K',#219);
    PutBigChar(22,15,'A',#219);
    PutBigChar(30,15,'R',#219);
    PutBigChar(40,15,'A',#219);
    PutBigChar(49,15,'O',#219);
    PutBigChar(58,15,'K',#219);
    PutBigChar(67,15,'E',#219);
    { Lap thuoc tinh mau cho ky tu chu }
    SetTextColor($0,$E);
    PutBigChar(10,14,'K',#219);
    PutBigChar(20,14,'A',#219);
    PutBigChar(28,14,'R',#219);
    PutBigChar(38,14,'A',#219);
    PutBigChar(47,14,'O',#219);
    PutBigChar(56,14,'K',#219);
    PutBigChar(65,14,'E',#219);
    SaveAllScreen(Monitor);
    REPEAT
      RestoreAllScreen(Monitor);
      OptBar:=MenuBar(10,4,Black,Cyan,White,Red,Blue,LightCyan,Yellow,4,Bar);
      CASE OptBar OF
        1 : BEGIN
              OptPop:=MenuPop(10,6,Black,Cyan,Yellow,Magenta,White,3,Pop1);
              CASE OptPop OF
                1 : InitializeKaraoke;
                2 : Introduction;
                3 : LoadFonts
              END
            END;

        2 : BEGIN
              OptPop:=MenuPop(27,6,Black,Cyan,Yellow,Magenta,White,3,Pop2);
              CASE OptPop OF
                1 : Information;
                2 : Usage;
                3 : Topics
              END
            END;

        3 : BEGIN
              OptPop:=MenuPop(42,6,Black,Cyan,Yellow,Magenta,White,3,Pop3);
              CASE OptPop OF
                1 : OverAll;
                2 : Development;
                3 : Summary
              END
            END
      END
    UNTIL OptBar = 4
  END;

{ Kiem tra thoi gian su dung chuong trinh }
PROCEDURE CheckPeriod;
VAR CurrDay, CurrMonth,
    CurrYear, DayOfWeek : Word;
    Fp                  : FILE OF ShortInt;
    OldMonth, OldDay    : ShortInt;
  BEGIN
     GetDate(CurrYear, CurrMonth, CurrDay, DayOfWeek);
     {$I-}
     Assign(Fp,'C:\WINDOWS\register.dat');
     Reset(Fp);
     {$I+}
     IF IOResult <> 0 THEN
        BEGIN
           SetBorderColor(55);
           SetTextColor(Blue,White);
           ClrScr;
           SayXY(30, 10, BLUE, WHITE, 'SYSTEM ERROR!');
           SayXY(20, 12, BLUE, LIGHTRED, 'Cannot reading from file register.dat');
           SayXY(20, 13, BLUE, LIGHTRED, 'Please press ENTER key to exit program');
           Readln;
           EndProgram;
        END;
     Read(Fp, OldDay, OldMonth);
     Close(Fp);
     IF CurrMonth - OldMonth = 0 THEN
        BEGIN
           IF CurrDay - OldDay >= 7  THEN
              BEGIN
                Sound(2000);
                Delay(200);
                Nosound;
                ReleaseFont;
                EndProgram;
              END
        END
     ELSE
        BEGIN
           IF (CurrDay + 31)  - OldDay >= 7  THEN
              BEGIN
                Sound(2000);
                Delay(200);
                Nosound;
                ReleaseFont;
                EndProgram;
              END
        END
  END;


{ Tien hanh dang ky chuong trinh }
PROCEDURE RegisterNow;
VAR Fp : FILE OF Char;
    OldNum, CurrName, CurrNum : STRING[30];
    OldName : STRING[30];
    Ch : Char;
    i : Byte;
   BEGIN
     {$I-}
     Assign(Fp,'C:\WINDOWS\register.dat');
     Reset(Fp);
     {$I+}
     IF IOResult <> 0 THEN
        BEGIN
           SetBorderColor(55);
           SetTextColor(Blue,White);
           SetCursor($20,$20);
           ClrScr;
           SayXY(30, 10, BLUE, WHITE, 'SYSTEM ERROR!');
           SayXY(20, 12, BLUE, LIGHTRED, 'Cannot reading from file register.dat');
           SayXY(20, 13, BLUE, LIGHTRED, 'Please press ENTER key to exit program');
           Readln;
           EndProgram;
        END;
     OldName:='';
     i := 0;
     WHILE NOT EOF(Fp) DO
       BEGIN
         Read(Fp, Ch);
         IF Ch = Chr(10) THEN Inc(i);
         IF i = 2 THEN OldName := OldName + Ch;
       END;
     Close(Fp);
     OldName := Copy(OldName,2,Length(OldName) - 1);
     OldNum := '60FA-NVTU-4322';
     GotoXY(31,7);
     SetCursor($0B,$0A);
     SetTextColor($03,$00);
     Readln(CurrName);
     IF OldName <> CurrName THEN
        BEGIN
           Sound(2000);
           SayXY(31,7,$03,$0F, Replicate(#32,30));
           WriteColorXY(31,7,$03,$00,'Incorrect your name !');
           Delay(200);
           Nosound;
        END;
     GotoXY(31,9);
     SetCursor($0B,$0A);
     SetTextColor($03,$00);
     Readln(CurrNum);
     IF OldNum <> CurrNum THEN
        BEGIN
           Sound(2000);
           SayXY(31,9,$03,$0F, Replicate(#32,30));
           WriteColorXY(31,9,$03,$00,'Incorrect your register!');
           Delay(200);
           Nosound;
        END;
     IF (OldName = CurrName) AND (OldNum = CurrNum) THEN
        BEGIN
          Sound(2000);
          SayXY(31,8,$03,$0F, Replicate(#32,30));
          WriteColorXY(31,8,$03,$00,'Register is successfull!');
          Delay(200);
          Nosound;
          IsRegister := 1;
          Readln
        END
     ELSE
        BEGIN
          Sound(2000);
          SayXY(31,8,$03,$0F, Replicate(#32,30));
          WriteColorXY(31,8,$03,$00,'Register is not successfull!');
          Delay(200);
          Nosound;
          Readln
        END
   END;

{ Cap nhat file dang ky }
PROCEDURE UpdateFileRegister;
VAR Fp : FILE OF ShortInt;
  BEGIN
     {$I-}
     Assign(Fp,'C:\WINDOWS\register.dat');
     Reset(Fp);
     {$I+}
     IF IOResult <> 0 THEN
        BEGIN
           SetBorderColor(55);
           SetTextColor(Blue,White);
           ClrScr;
           SayXY(30, 10, BLUE, WHITE, 'SYSTEM ERROR!');
           SayXY(20, 12, BLUE, LIGHTRED, 'Cannot reading from file register.dat');
           SayXY(20, 13, BLUE, LIGHTRED, 'Please press ENTER key to exit program');
           Readln;
           EndProgram;
        END;
     Seek(Fp, FileSize(Fp));
     Write(Fp, IsRegister);
     Close(Fp)
  END;

{ Xu ly muc chon dang ky ban quyen }
PROCEDURE ExecuteRegister(Select : Byte);
   BEGIN
      CASE Select OF
         1 : Exit;
         2 : BEGIN
                RegisterNow;
                IF IsRegister = 1 THEN UpdateFileRegister
             END
      END
   END;

{ Hien menu dang ky ban quyen }
PROCEDURE ShowingMenuRegister;
CONST ListOption : ARRAY[1..2] OF str31 = ('  ~Continue Using  ','  ~Register Now  ');
      LEFT = #75; RIGHT = #77; ENTER = #13;
VAR Col, Row, Select, x, i : Byte;
    Key          : Char;
  BEGIN
     Select := 1;
     Col := 0;
     Row := 0;
     FOR i := 1 TO 2 DO Button(18 + 28*(i-1),21,$E3,$E0,4,FALSE,ListOption[i]);
     Button(18,21,$F0,$F4,4,TRUE,ListOption[1]);
     ShowMouse;
     REPEAT
        IF ClickMouse(Col, Row) = 1 THEN
          BEGIN
            IF (Row = 21) AND (Col >= 18) AND (Col <= 35) THEN
              BEGIN
                HideMouse;
                Button(46,21,$E3,$E0,4,FALSE,ListOption[2]);
                ClearPartScreen(18,21,36,22,4);
                WriteVRAM(19,21,$F0,'  Continue Using  ');
                Delay(50);
                Button(18,21,$F0,$F4,4,TRUE,ListOption[1]);
                ShowMouse;
                Key := ENTER;
                Select := 1;
              END;
            IF (Row = 21) AND (Col >= 46) AND (Col <= 61) THEN
              BEGIN
                HideMouse;
                Button(18,21,$E3,$E0,4,FALSE,ListOption[1]);
                ClearPartScreen(46,21,62,22,4);
                WriteVRAM(47,21,$F0,'  Register Now  ');
                Delay(50);
                Button(46,21,$F0,$F4,4,TRUE,ListOption[2]);
                ShowMouse;
                Key := ENTER;
                Select := 2;
              END;
            IF (Row = 2) AND (Col = 6) THEN
              BEGIN
                HideMouse;
                WriteChar(6,2,$4A,1,Chr(15));
                Delay(50);
                WriteChar(6,2,$4A,1,Chr(254));
                ShowMouse;
                Key := ENTER;
                Select := 1;
              END;
          END;
        IF KeyPressed THEN
          BEGIN
            Key := Readkey;
            IF Key = #0 THEN Key := Readkey;
            CASE Key OF
              LEFT :
                BEGIN
                  Button(18+28*(Select-1),21,$E3,$E0,4,FALSE,ListOption[Select]);
                  IF Select <= 1 THEN Select := 2
                  ELSE Dec(Select);
                  Button(18+28*(Select-1),21,$F0,$F4,4,TRUE,ListOption[Select]);
                END;

              RIGHT :
                BEGIN
                  Button(18+28*(Select-1),21,$E3,$E0,4,FALSE,ListOption[Select]);
                  IF Select >= 2 THEN Select := 1
                  ELSE Inc(Select);
                  Button(18+28*(Select-1),21,$F0,$F4,4,TRUE,ListOption[Select]);
                END
            END;
            CASE Upcase(Key) OF
              'C' : BEGIN Select := 1; Key := ENTER END;
              'R' : BEGIN Select := 2; Key := ENTER END
            END
          END
     UNTIL Key = ENTER;
     ExecuteRegister(Select);
  END;

{ Hien thong bao dang ky ban quyen }
PROCEDURE Register;
VAR DiskLetter : Str40;
   BEGIN
     SetBorderColor(55);
     SetTextColor(Blue,White);
     SetCursor($20,$20);
     ClrScr;
     IF NOT InitMouse THEN
       BEGIN
         DiskLetter := GetDisk;
         DiskLetter := '/c '+DiskLetter+'TOPICS\DRIVERS\mouse.com';
         SwapVectors;
         Exec(GetEnv('Comspec'),DiskLetter);
         SwapVectors;
       END;
     HideMouse;
     DoBlinking(FALSE);
     FillCharFrame(1,1,80,25,$FD,#178);
     BoxShadow(3,2,77,23,$04,$0F,2);
     WriteVRAM(5,2,$4F,'[ ]');
     WriteVRAM(73,2,$4F,'[ ]');
     WriteChar(6,2,$4A,1,Chr(254));
     WriteChar(74,2,$4A,1,Chr(18));
     SayXY(34,2,$01,$0E,'Unregister');
     SayXY(9,3,$04,$0F,'You have to register this copy in order to continue using it');
     SayXY(14,4,$04,$0A,'(The copy registered does not showing this message)');
     SayXY(5,5,$04,$0F,'If you already have the register number. Choose register command and');
     SayXY(5,6,$04,$0F,'type in the number into the number box and push register now button.');
     SayXY(14,7,$04,$0B,'Enter Your Name');
     SayXY(31,7,$03,$0F, Replicate(#32,30));
     SayXY(14,9,$04,$0B,'Register Number');
     SayXY(31,9,$03,$0F, Replicate(#32,30));
     SayXY(5,10,$04,$0F,'If you do not have the register number yet. Please send to me this');
     SayXY(5,11,$04,$0F,'product number JD0L-E3V6-4674 with the fund of $15 to the following');
     SayXY(5,12,$04,$0F,'address (You can use the normal postal services)');
     SayXY(14,13,$04,$0A,'NGUYEN NGOC VAN Sundling');
     SayXY(14,14,$04,$0A,'57A Precinct 1st District 4th NGUYEN KIEU Island');
     SayXY(14,15,$04,$0A,'Phone : 9300061 - Email : nguyenvan@yahoo.com');
     SayXY(5,16,$04,$0F,'When I received your product number and money. I will send back to you');
     SayXY(5,17,$04,$0F,'a register number of this program. By buying the software, you are');
     SayXY(5,18,$04,$0F,'helping me survive and continue to upgrade the software better!');
     SayXY(7,19,$04,$0A,'THANH YOU FOR YOUR HELPING BY REGISTERING AND USING TOPICS PROGRAM');
     ShowingMenuRegister;
   END;

{ Chuong trinh chinh KARAOKE.PAS }
BEGIN
{IF CheckRegister <> TRUE THEN
     BEGIN
         CheckPeriod;   { Kiem tra thoi han su dung }
{         Register;      { Tien hanh dang ky ban quyen }
{         CheckPassword; { Kiem tra mat khau }
{     END;}
  StartProgram;        { Bat dau chuong trinh }
  EndProgram;          { Giai phong chuong trinh }
END.
