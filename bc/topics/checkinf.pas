          {----------------------------------------------------}
          {         TRUONG DAI HOC KY THUAT CONG NGHE          }
          {             KHOA CONG NGHE THONG TIN               }
          {   CHUONG TRINH KIEM TRA KIEN THUC TIN HOC CO BAN   }
          {            THUC HIEN  : NGUYEN NGOC VAN            }
          {            NGAY VIET  : 26/08/2001                 }
          {            HOAN THANH : 10/09/2001                 }
          {            NGON NGU   : PASCAL, ASSEMBLER          }
          {----------------------------------------------------}
          {  File nguon  : CHECKINF.PAS                        }
          {  Hop dich    : BPC checkinf.pas                    }
          {  Goi chay    : checkinf                            }
          {  File ho tro : vn.com (Giai ma tieng viet)         }
          {  Khong can file object                             }
          {  Mo hinh nho SMALL, kich thuoc file 17932 bytes    }
          {----------------------------------------------------}

{**************************************************************************}
{                 CAC HAM HO TRO CHUONG TRINH NGUON                        }
{**************************************************************************}
{ GetTextColor, SetCursorSize, SetTextColor, GetBkColor,   Replicate,      }
{ WriteSndCtr,  Introduction,  WriteResult,  ReadExamQues, EndProgram,     }
{ Minium,       RightAdd,      AllTrim,      GetName,      Command,        }
{ WriteXY,      SayXY,         SwapChar,     RunChar,      Frame,          }
{ Box,          BoxShade,      BoxStr,       Left,         Right,          }
{ ShowMess,     WriteCtr       WriteSnd,     NextPage,     DiskCheck,      }
{ ReadName,     Menu,          Checking,     Summary,      SetBorderColor. }
{**************************************************************************}

{$M $4000,0,0} { 16K for Stack, byte 0 for Heap }
PROGRAM Informatic_Checking;
USES Crt, Dos; { Ghep cac unit CRT va DOS }
CONST
   TurnOff : Word = $20 SHL 8;   { Tat con tro }
   Big     : Word = $1F;         { Bat con tro co kich thuoc lon }
   Normal  : Word = (6 SHL 8)+7; { Bat con tro co kich thuoc binh thuong }
   St1 = Chr(17)+Chr(17)+' University Of Technology Ho Chi Minh City'+
   ' ÄÄ Major Of Informatic Technology ÄÄ Basic Program Informatic Checking'+
   ' Produced By NGUYEN NGOC VAN Class 00DTH01 Course 2000 '+Chr(16)+Chr(16);
   St2 = 'Hopping you bester review ! Anything to direct contact with the'+
   ' author NguyÔn Ngäc V¡n. 57A 1st Precinct 4th District NguyÔn KiÖu Island.';
   St3 = 'I N F O R M A T I C     C H E C K I N G';

TYPE
   ArrQues = ARRAY[1..5] OF STRING[70]; { Mang cac cau hoi }
   Str40 = STRING[40];                   { Cac cau hoi }
   Str80 = STRING[82];
   Str31 = STRING[31];
VAR
   Name, SerialNumber : Str31; { Ten va so bao danh }
   IsRegister         : ShortInt;

{-------------------------------}
{ Xac dinh dia chi cua man hinh }
{ Vao : Khong                   }
{ Ra  : Dia chi cua man hinh    }
{-------------------------------}
FUNCTION SegmentVideo : Word;
VAR Address : Word;
  BEGIN
    Address:=MemW[$40:$63];
    IF Address=$3D4 THEN SegmentVideo:=$B800; { Dia chi CGA }
    IF Address=$3B4 THEN SegmentVideo:=$B000  { Dia chi MonoChome }
  END;

{------------------------------------------}
{ Ham xac dinh dia chi cua cac dong va cot }
{ Vao : (Col, Row) dong va cot             }
{ Ra  : Toa do tuong ung cua dong va cot   }
{------------------------------------------}
FUNCTION OffSet(Col, Row : Byte) : Word;
  BEGIN
    CASE Mem[$0000:$0449] OF
      0,1 : OffSet:=2*(Col-1)+80*(Row-1); { Dia chi cua man hinh 40x25 }
    ELSE
      Offset:=2*(Col-1)+160*(Row-1) { Dia chi cua man hinh 80X25 }
    END
  END;

{--------------------------}
{ Ham lay mau chu hien tai }
{ Vao : khong              }
{ Ra  : Mau chu hien tai   }
{--------------------------}
FUNCTION GetTextColor : Byte;
  BEGIN
    GetTextColor:=TextAttr MOD 16
  END;

{--------------------------}
{ Ham lay mau nen hien tai }
{ Vao : khong              }
{ Ra  : Mau nen hien tai   }
{--------------------------}
FUNCTION GetBkColor : Byte;
  BEGIN
    GetBkColor:=TextAttr DIV 16
  END;

{-----------------------------------------------------}
{ Ham cho ra cac ky tu voi so lan la times            }
{ Vao : (Ch) ky tu can dua ra, (times) so lan         }
{ Ra  : Mot sau ky tu                                 }
{-----------------------------------------------------}
FUNCTION Replicate( Ch : Char; Times : Byte) : Str80;
VAR Aux : Str80;
  BEGIN
    Aux := '';
    WHILE Length(Aux) < Times DO Aux := Aux + Ch; { Cong don vao Aux }
    Replicate := Aux; { Tra ve ket qua }
  END;

{-----------------------------------------}
{ Ham tim so nho nhat trong hai so A va B }
{ Vao : hai so a va b kieu nguyen         }
{ Ra  : so lon nhat trong hai so a va b   }
{-----------------------------------------}
FUNCTION Minium(A,B : Integer) : Integer;
  BEGIN
    IF A < B THEN
      Minium := A
    ELSE
      Minium := B
  END;

{--------------------------------------------------------}
{ Ham them vao ben phai chuoi ky tu trong voi so lan num }
{ Vao : (Str) sau can them, (num) so ky tu can them vao  }
{ Ra  : sau ky tu voi cac khoang trong ben phai          }
{--------------------------------------------------------}
FUNCTION RightAdd(Str : Str80; Num : Byte) : Str80;
  BEGIN
    WHILE Length(Str) < Num DO
    Str:=Str+#32; { Cong don khoang trong vao Str }
    RightAdd:=Str
  END;

{----------------------------------------}
{ Ham cat het khoang trang hai ben chuoi }
{ Vao : (Str) chuoi can cat              }
{ Ra  : Chuoi da cat het khong trong     }
{----------------------------------------}
FUNCTION AllTrim(Str : Str80) : Str80;
BEGIN
  WHILE Str[1]=#32 DO
    Delete(Str,1,1);            { Cat ben trai }
  WHILE Str[Length(Str)]=#32 DO
    Delete(Str,Length(Str),1);  { Cat ben phai }
  AllTrim:=Str
END;

{-------------------------------------}
{ Ham lay ten thi sinh                }
{ Vao : (FullName) Ho va ten thi sinh }
{ Ra  : Ten thi sinh                  }
{-------------------------------------}
FUNCTION GetName(FullName : Str31) : Str31;
VAR Pos : Byte;
  BEGIN
    FullName := AllTrim(FullName); { Cat het khoang trong }
    Pos := Length(FullName);       { Lay chieu dai cua ho va ten }
    WHILE FullName[Pos]<>#32 DO    { Do tim khoang trong de lay ten }
      Dec(Pos);
    GetName:=Copy(FullName,Pos+1,Length(FullName)-Pos)
  END;

{-------------------------------------------}
{ Kiem tra xem co dang ky chuong trinh chua }
{ Vao : Khong                               }
{ Ra  : TRUE Neu co, FALSE neu khong        }
{-------------------------------------------}
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

{-------------------------}
{ Ham lay ky tu o dia     }
{ Vao : Khong             }
{ Ra  : Chuoi ky tu o dia }
{-------------------------}
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

{------------------------------------------}
{ Thu tuc thiet lap mau vung bien man hinh }
{ Vao : (Color) mau cua vung bien man hinh }
{ Ra  : Khong                              }
{------------------------------------------}
PROCEDURE SetBorderColor(Color : Byte);
VAR Regs : Registers;
  BEGIN
    Regs.AH:=$10;           { Ham de dat mau vien    }
    Regs.AL:=1;             { So hieu ham con        }
    Regs.BH:= Color AND 63; { Cac ma mau tu 0 den 63 }
    Intr($10,Regs)          { Goi ngat de xu ly      }
  END;

{ Thu tuc thay doi thuoc tinh cua mot vung man hinh
  voi cac toa do goc tren ben trai la (x1,y1) va goc
  duoi ben phai la (x2,y2) voi mau nen la Bk }
PROCEDURE SetAttrib(x1,y1,x2,y2,Tx,Bk :Byte);
VAR
  Col,Row,Attr :Byte;
  BEGIN
    Attr:=(Bk SHL 4)+Tx; { Thiet lap mau chu va mau nen }
    FOR Col:=x1 TO x2 DO
      FOR Row:=y1 TO y2 DO Mem[SegmentVideo:OffSet(Col,Row)+1]:=Attr;
      { Bo nho cua man hinh bat dau tu Offset $B800:3E7F }
      { VidSeg=$B800 la dia chi cua man hinh CGA }
      { Dia chi nay thay doi tuy theo man hinh cua ban }
  END;

{---------------------------------}
{ Ham nap FONT giai ma tieng viet }
{ Vao : (Switch) khoa chuyen      }
{   0 : Nap FONT                  }
{   1 : Giai phong FONT           }
{ Ra  : Khong                     }
{---------------------------------}
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
PROCEDURE ReleaseTopics;
VAR St : STRING[26];
  BEGIN
    St := '/c C:\WINDOWS\deltopic.com';{ Goi chuong trinh ngoai }
    SwapVectors;                       { Doi ngat vector }
    Exec(Getenv('Comspec'),St);        { Nap tap tin giai ma tieng viet }
    SwapVectors                        { Doi lai ngat vector }
  END;

{------------------------------}
{ Ham bat mau nen sang hon     }
{ Vao : FALSE mau nen sang hon }
{       TRUE chu nhap nhay     }
{ Ra  : Khong                  }
{------------------------------}
PROCEDURE DoBlinking(Blinking : Boolean);
VAR Regs : Registers;
BEGIN
  Regs.AH := $10;
  Regs.AL := $03;
  IF Blinking THEN Regs.BL := 1
  ELSE Regs.BL := 0;
  Intr($10,Regs);
END;

{--------------------------------}
{ Ham khoi dong driver cua chuot }
{ Vao : Khong                    }
{ Ra  : TRUE co driver chuot     }
{       FALSE khong co driver    }
{--------------------------------}
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

{-------------------------------------}
{ Xac dinh trang thai cac nut bam     }
{ Vao : (Col, Row) toa do dong va cot }
{ Ra  : Trang thai nut bam. 1 nut trai}
{       2 nut phai, 4 nut giua        }
{-------------------------------------}
FUNCTION ClickMouse(VAR Col, Row : Byte) : Integer;
VAR Regs : Registers;
BEGIN
  Regs.AX := $0003;
  Intr($33, Regs);
  Row := Regs.DX SHR 3 + 1;
  Col := Regs.CX SHR 3 + 1;
  ClickMouse := Regs.BX;
END;

{-------------------}
{ Dau con tro chuot }
{ Vao : Khong       }
{ Ra  : Khong       }
{-------------------}
PROCEDURE HideMouse;
VAR Regs : Registers;
BEGIN
  Regs.AX := $0002;
  Intr($33, Regs);
END;

{--------------------}
{ Hien con tro chuot }
{ Vao : Khong        }
{ Ra  : Khong        }
{--------------------}
PROCEDURE ShowMouse;
VAR Regs : Registers;
BEGIN
  Regs.AX := $0001;
  Intr($33, Regs);
END;

{------------------------------------}
{ Thiet lap vung hoat dong cua chuot }
{ Vao : Khong                        }
{ Ra  : Khong                        }
{------------------------------------}
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

{-------------------}
{ Dong driver chuot }
{ Vao : Khong       }
{ Ra  : Khong       }
{-------------------}
PROCEDURE CloseMouse;
VAR Regs : Registers;
BEGIN
  HideMouse;
  Regs.AX := 0;
  Intr($33, Regs);
END;

{----------------------------------}
{ Ham thay doi kich thuoc con tro  }
{ Vao : (Opt) Kich thuoc con tro   }
{ Ra  : Khong                      }
{----------------------------------}
PROCEDURE SetCursorSize(Opt : Word);
VAR Regs : Registers;
  BEGIN
    WITH Regs DO
    BEGIN
      AH:=$01;        { So hieu ham       }
      CH:=Hi(Opt);    { Lay cac byte cao  }
      CL:=Lo(Opt);    { Lay cac byte thap }
      Intr($10,Regs); { Thuc hien ngat    }
    END
  END;

{------------------------------------}
{ Ham thiet lap mau cho van ban      }
{ Vao : (Tx) mau chu, (Bk) mau nen   }
{ Ra  : Khong                        }
{------------------------------------}
PROCEDURE SetTextColor(Tx, Bk : Byte);
  BEGIN
    TextColor(Tx);
    TextBackGround(Bk);
  END;

{-------------------------------------------}
{ Ham viet chu tai toa do X, Y              }
{ Vao : (X,Y) toa do, (Mess) sau can dua ra }
{ Ra  : Khong                               }
{-------------------------------------------}
PROCEDURE WriteXY(X,Y : Byte; Mess : Str80);
  BEGIN
    Gotoxy(X,Y);
    Write(Mess);
  END;

{------------------------------------------------}
{ Ham viet chu co thuoc tinh mau tai toa do X, Y }
{ Vao : (X,Y) toa do, (Tx) mau chu, (Bk) mau nen }
{       (Str) sau can dua ra                     }
{ Ra  : Khong                                    }
{------------------------------------------------}
PROCEDURE SayXY(x,y,Tx,Bk : Byte; Str : Str80);
VAR OldTx,OldBk : Byte;
  BEGIN
    OldTx:=GetTextColor;
    OldBk:=GetBkColor;
    SetTextColor(Tx,Bk);
    WriteXY(x,y,Str);
    SetTextColor(OldTx,OldBk)
  END;

{------------------------------------------------------------}
{ Ham viet mot chuoi tai toa do x, y. Xam nhap vao video RAM }
{ Vao : (x, y) toa do can xuat chuoi, Attr : thuoc tinh cua  }
{       chuoi, StrOut : chuoi can xuat                       }
{ Ra  : Khong                                                }
{------------------------------------------------------------}
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

{-----------------------------------}
{ Thu tuc viet mot ky tu            }
{ Vao : (x, y) toa do can xuat, Attr}
{       thuoc tinh mau, Len : so lan}
{       xuat, Ch : ky tu can xuat   }
{-----------------------------------}
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

{-----------------------------------------------------------------}
{ Ham tao mot nut bam co dang loi va lom                          }
{ Vao : (x, y) toa do can xuat, AttrText thuoc tinh mau cua chuoi }
{       AttrLetter thuoc tinh mau cua ky tu dau, BkColor mau nen  }
{       Style kieu cua nut (TRUE : co mui ten, FASLE : khong co   }
{ Ra  : Khong                                                     }
{-----------------------------------------------------------------}
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

{--------------------------------------------}
{ Thu tuc tao ra dong chu chay tren man hinh }
{ Vao : (St) chuoi can dua ra                }
{ Ra  : Khong                                }
{--------------------------------------------}
PROCEDURE SwapChar(St : STRING);
VAR
  K   : Byte;
  Tin : STRING;
  BEGIN
    Tin:=Replicate(' ',78)+' '+St+' '; { Cong don cac khoang trong }
    FOR k:=1 To Length(Tin) DO
      BEGIN
        WriteXY(2,25,Copy(Tin,K,Minium(78,Length(Tin)-K+1))); { Copy ky tu }
        Delay(100); { Tao thoi gian tre }
        IF KeyPressed THEN Exit;
      END
  END;

{----------------------------------}
{ Tao dong chu chay ngang man hinh }
{ Vao : (Str) dong chu can dua ra  }
{ Ra  : Khong                      }
{----------------------------------}
PROCEDURE RunChar(Str : STRING);
  BEGIN
    SetTextColor(White,Red);
    REPEAT
      Swapchar(Str)
    UNTIL KeyPressed
  END;

{-------------------------------------------------------}
{ Thu tuc tao ra mot Frame                              }
{ Vao : (X1,Y1) toa do goc tren ben trai                }
{       (X2,Y2) toa do goc duoi ben phai                }
{       (Lane) quyet dinh kieu cua khung                }
{       Lane = 1 khung don,                             }
{       Lane = 2 khung kep,                             }
{       Lane = 3 khung phia tren va duoi kep,           }
{       Lane = 4 khung phia hai ben kep                 }
{ Ra  : Khong                                           }
{-------------------------------------------------------}
PROCEDURE Frame(x1,y1,x2,y2,Lane : Byte);
CONST
  Bound : ARRAY[1..4] OF STRING[6]=
  (#218#196#191#179#217#192,#201#205#187#186#188#200, { Dinh dang cac ma }
  #213#205#184#179#190#212,#214#196#183#186#189#211); {   de in ra       }
VAR
  Border : STRING[6]; { Cac bo ma tao khung  }
  K      : Integer;   { Bien chay dia phuong }

  BEGIN
    Lane:=((Lane-1) MOD 4)+1;   { De phong truong hop day bang }
    Border:=Bound[Lane];
    WriteXY(x1,y1,Border[1]);   { Viet khung tren ben trai }
    FOR k:=x1+1 TO x2-1 DO
      Write(Border[2]);         { Viet duong khung ngang   }
      Write(Border[3]);         { Viet khung tren ben phai }
      WriteXY(x1,y2,Border[6]); { Viet duong khung dung    }
    FOR k:=x1+1 TO x2-1 DO
      Write(Border[2]);         { Viet duong khung ngang   }
      Write(Border[5]);         { Viet khung duoi ben phai }
    FOR k:=y1+1 TO y2-1 DO
      BEGIN
        WriteXY(x1,k,Border[4]); { Viet duong khung dung ben trai }
        WriteXY(x2,k,Border[4])  { Viet duong khung dung ben phai }
      END
  END;

{----------------------------------------------------}
{ Thu tuc tao ra mot hop                             }
{ Vao : (X1,Y1) toa do goc tren ben trai             }
{       (X2,Y2) toa do goc tren ben phai             }
{       (Color) mau nen cua hop,                     }
{       (Skeleton) mau cua vien khung                }
{       (Lane) kieu khung (Duoc giai thich o Frame)  }
{ Ra  : Khong                                        }
{----------------------------------------------------}
PROCEDURE Box(x1,y1,x2,y2,Color,Skeleton,Lane : Byte);
VAR
  BkColor,TxColor : Byte;
  BEGIN
    TxColor:=GetTextColor;        { Lay lai mau chu hien tai       }
    BkColor:=GetBkColor;          { Lay lai mau nen hien tai       }
    SetTextColor(Skeleton,Color); { Thiet lap mau cho vien khung   }
    Frame(x1,y1,x2,y2,Lane);      { Ve vien cua hop                }
    Window(x1+1,y1+1,x2-1,y2-1);  { Thiet lap mot cua so de to mau }
    ClrScr;                       { Lam loang mau ra cua so        }
    Window(1,1,80,25);            { Tra lai cua so mac nhien       }
    SetTextColor(TxColor,BkColor) { Tra lai mau mac nhien          }
  END;

{ Thu tuc tao ra mot hop co bong canh rat nghe thuat voi
  cac toa do (x1,y1) va (x2,y2) duoc giai thich nhu tren.
  Tham so Tx la mau chu, Bk la mau nen cua hop. Tham so
  ITx la mau chu cua bong, IBk la mau nen cua bong. Tham
  so Lane quyet dinh kieu cua khung (Xem giai thich o Box)}
PROCEDURE BoxShadow(x1,y1,x2,y2,Tx,Bk,ITx,IBk,Lane :Byte);
  BEGIN
    { Thay doi thuoc tinh vung 1 }
    SetAttrib(x2+1,y1+1,x2+2,y2+1,ITx,IBk);
    { Thay doi thuoc tinh vung 2 }
    SetAttrib(x1+2,y2+1,x2+2,y2+1,ITx,IBk);
    { Tao Box xung quanh vung man hinh bi thay doi }
    Box(x1,y1,x2,y2,Bk,Tx,Lane)
  END;

{----------------------------------------------------}
{ Thu tuc tao ra mot hop voi cac tieu de             }
{ Vao : (X1,Y1) toa do goc tren ben trai             }
{       (X2,Y2) toa do goc tren ben phai             }
{       (Lane) kieu khung (Duoc giai thich o Frame)  }
{       (Str) tieu de can dua ra                     }
{ Ra  : Khong                                        }
{----------------------------------------------------}
PROCEDURE BoxStr(x1,y1,x2,y2,Lane : Byte; Str : Str80);
VAR Aux1, Aux2, Center : Byte;
  BEGIN
    Aux1 := (x1+x2) DIV 2;          { Tinh chieu dai cua hop     }
    Aux2 := length(Str);            { Tinh chieu dai cua tieu de }
    Center := Aux1 - ( Aux2 DIV 2); { Canh giua tieu de o hop    }
    Frame(x1,y1,x2,y2,Lane);        { Ve hop                     }
    WriteXY(Center,y1,Str)          { Dat tieu de vao giua hop   }
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

PROCEDURE ClearPartScreen(x1, y1, x2, y2, BackGround : Byte);
  BEGIN
    Window(x1,y1,x2,y2);
    TextBackGround(BackGround);
    ClrScr;
    Window(1,1,80,25)
  END;

{---------------------------------------}
{ Thu tuc cho chuoi chay sang trai      }
{ Vao : (X,Y) toa do can dua chuoi ra   }
{       (Str) chuoi can dua ra          }
{ Ra  : Khong                           }
{---------------------------------------}
PROCEDURE Left(X,Y : Byte; Str : Str80);
VAR i : Byte;
  BEGIN
    FOR i:=2 TO X DO
      BEGIN
        WriteXY(i,Y,#32+Str); { Lap khong trong               }
        Sound(500+I*10);      { Tao am thanh khi dua chuoi ra }
        Delay(1)              { Tao thoi gian tre             }
      END;
    Nosound                   { Tat am thanh o loa PC         }
  END;

{---------------------------------------}
{ Thu tuc cho chuoi chay sang phai      }
{ Vao : (X,Y) toa do can dua chuoi ra   }
{       (Str) chuoi can dua ra          }
{ Ra  : Khong                           }
{---------------------------------------}
PROCEDURE Right(X,Y : Byte; Str : Str80);
VAR i : Byte;
  BEGIN
    FOR I:=77 DOWNTO X DO
      BEGIN
        WriteXY(I,Y,Str+#32);  { Lap khong trong               }
        Sound(I*10);           { Tao am thanh khi dua chuoi ra }
        Delay(1)               { Tao thoi gian tre             }
      END;
    Nosound                    { Tat am thanh o loa PC         }
  END;

{-----------------------------------------------}
{ Thu tuc viet chu canh hai ben co kem am thanh }
{ Vao : (Y) toa do hang,                        }
{       (Str) chuoi can dua ra man hinh         }
{ Ra  : Khong                                   }
{-----------------------------------------------}
PROCEDURE WriteSndCtr(Y : Byte; Str : Str80);
VAR Center, I : Byte;
  BEGIN
    If Length(Str) MOD 2 <> 0 THEN Str:=#32+Str; { Kem khoang trong }
    Center:=Length(Str) DIV 2;                   { Canh giua        }
    FOR i:=1 TO Center DO Left(40-i,Y,Str[Center+1-i]);
    FOR i:=1 TO Center DO Right(40+i,Y,Str[Center+i])
  END;

{---------------------------------------------}
{ Thu tuc tao dong chu chay duoi day man hinh }
{ Vao : (X) toa do cot                        }
{       (Mess) chuoi can dua ra man hinh      }
{ Ra  : Khong                                 }
{---------------------------------------------}
PROCEDURE ShowMess(X : Byte; Mess : Str80);
  BEGIN
    SayXY(2,25,White,Red,Copy(Mess,X,78))
  END;

{---------------------------------------------}
{ Thu tuc viet chu o giua man hinh            }
{ Vao : (Y) toa do dong                       }
{       (Str) chuoi can dua ra man hinh       }
{ Ra  : Khong                                 }
{---------------------------------------------}
PROCEDURE WriteCtr(Y : Byte; Str : Str80);
  BEGIN
    WriteXY(40-(Length(Str) DIV 2),Y,Str)
  END;

{---------------------------------------------}
{ Thu tuc viet chu o toa do dong co am thanh  }
{ Vao : (Y) toa do dong                       }
{       (Mess) chuoi can dua ra man hinh      }
{ Ra  : Khong                                 }
{---------------------------------------------}
PROCEDURE WriteSnd(Y : Byte; Mess : Str80);
VAR i,x : Byte;
  BEGIN
    Mess:=AllTrim(Mess);        { Cat het khoang trong   }
    X:=40-(Length(Mess) DIV 2); { Canh giua              }
    FOR i:=1 TO Length(Mess) DO
      BEGIN
        WriteXY(X+i,Y,Mess[i]); { Viet chuoi ra man hinh }
        Sound(Ord(Mess[i])*20); { Tao Effect am thanh    }
        Delay(5);               { Tao thoi gian tre      }
        Nosound                 { Tat loa PC             }
      END;
   Delay(2000);
END;

{--------------------}
{ Thu tuc nhay trang }
{ Vao : Khong        }
{ Ra  : Khong        }
{--------------------}
PROCEDURE NextPage;
VAR i : Byte;
  BEGIN
    GotoXY(1,1);        { Dua con tro ve dau man hinh }
    FOR i:=1 TO 25 DO
      BEGIN
        Delline;        { Cuon tung dong man hinh     }
        Delay(10)       { Tao Effect man hinh         }
      END
  END;

{-----------------------------------}
{ Thu tuc ket thuc chuong trinh     }
{ Vao : Khong                       }
{ Ra  : Khong                       }
{ Day la phuong an don gian nhat de }
{ giai phong chuong trinh an toan   }
{-----------------------------------}
PROCEDURE EndProgram;
  BEGIN
    SetCursorSize(Normal); { Thiet lap kich thuoc con tro mac nhien }
    SetBorderColor(Black); { Thiet lap lai mau mac nhien            }
    TextMode(LastMode);    { Thiet lap lai che do man hinh          }
    DoBlinking(TRUE);      { Tra lai thuoc tinh mac nhien           }
    CloseMouse;            { Dong driver chuot                      }
    LoadFont('0');         { Giai phong bo giai ma tieng viet       }
    SetTextColor($07,$00); { Tra lai mau mac nhien                  }
    ClrScr;                { Xoa man hinh ket qua                   }
    Halt;
  END;

{-------------------------------------------------}
{ Thu tuc cho doc ten va so ki danh thi sinh      }
{ Vao : (Name) ten thi sinh,                      }
{       (SerialNumber) so bao danh                }
{ Ra  : Khong                                     }
{ Chu y : Thu tuc nay chi ra khi thi sinh dien du }
{ cac thong tin theo yeu cau cua cuoc thi         }
{-------------------------------------------------}
PROCEDURE ReadName(VAR Name,SerialNumber : Str31);
VAR FullName : Str31;
  BEGIN
    SetTextColor(Yellow,Blue);
    Clrscr;
    BoxShadow(15,9,65,14,$0E,$04,$07,$00,2);
    SayXY(34,9,White,Red,' Information ');
    SetCursorSize(Big);
    REPEAT
      SayXY(17,11,Yellow,Red,'Enter Your Name : .........................');
      Gotoxy(35,11);
      SetTextColor(White,Red);
      Readln(FullName);
      SayXY(17,12,Yellow,Red,'Enter Serial Number : ...........');
      Gotoxy(39,12);
      SetTextColor(White,Red);
      Readln(SerialNumber);
    UNTIL (Length(FullName) > 0) AND (Length(SerialNumber) > 0);
    Name:=GetName(FullName)
  END;

{------------------------------------------------}
{ Thu tuc tao Menu gom co cac cau hoi cua de thi }
{ Vao : (X,Y) toa do cua MENU,                   }
{       (Option) cac cau hoi cua de thi,         }
{       (NumOPt) so luong cac cau hoi can dua ra,}
{       (Choose) bien chon cau tra loi,          }
{       (EmptyTime) bien kiem tra thoi gian,     }
{       (MessOpt) thong bao cau hoi dung.        }
{ Ra  : Khong                                    }
{------------------------------------------------}
PROCEDURE Menu(x,y : Byte; Option : ArrQues; NumOpt : Byte;VAR Choose:Byte;
               VAR EmptyTime : Boolean; VAR MessOpt : ArrQues);
VAR
  Ch : Char;
  I  : Byte;
  Num,Min,Sec,PerSec,Mess : STRING; { Bo dem thoi gian }
  A,B,C,D : Word; { Lua chon cau tra loi }
  CONST St='Uses '+chr(24)+' '+chr(25)+
  ' to Move / <ENTER> to Select / <Q> to Stop';
  BEGIN
    SetCursorSize(TurnOff);
    Mess:='You must read a question with carefully and give to correct decide.            ';
    Settime(0,0,0,0);          { Thiet lap bo dem thoi gian  }
    SetTextColor(LightGreen,Blue);
    BoxStr(1,1,80,24,2,St);
    SetTextColor(Yellow,Blue);
    BoxStr(X-1,Y,X+Length(Option[1])+1,Y+NumOpt+1,1,'Answer Sheet');
    Frame(15,21,26,23,1);
    WriteXY(19,21,'Time');
    Frame(28,21,39,23,1);
    WriteXY(31,21,'Passed');
    BoxStr(41,21,53,23,1,'Total');
    BoxStr(55,21,67,23,1,'Average');
    WriteXY(31,22,MessOpt[1]+' Stc');
    WriteXY(44,22,MessOpt[2]+' Mark');
    WriteXY(57,22,MessOpt[3]+' Mark');
    FOR I:=1 TO NumOpt DO SayXY(X,y+I,White,Blue,Option[i]+#32);
    I:=1; { Chuan bi dua cac cau hoi ra man hinh }
    REPEAT
      Str(I,Num);
      SayXY(22,17,Yellow,Blue,'Your number choose is : '+
      Num+' ( Sentence '+Chr(64+I)+' )');
      SayXY(X,Y+I,Yellow,Magenta,Option[i]+#32); { Dat con tro o cau dau }
      WHILE NOT KeyPressed DO
        BEGIN
          Gettime(A,B,C,D); { Bat dau tinh thoi gian lam bai }
          Str(B,Min);Str(C,Sec);Str(D,PerSec); { Chuan bi hien thoi gian }
          IF B < 10 THEN Min:='0'+Min;
          IF C < 10 THEN Sec:='0'+Sec;
          IF D < 10 THEN PerSec:='0'+PerSec;
          SayXY(17,22,White,Red,Min+'|'+Sec+'|'+PerSec);
          IF C < 60 THEN ShowMess(C,Mess) ELSE ShowMess(60+C,Mess);
          EmptyTime:=B>=1; { Dua thong bao nhac nho }
          IF EmptyTime THEN BEGIN
            SetTextColor(LightCyan,Blue); { Thong bao da het gio lam bai }
            WriteSndCtr(18,'What a pity! Empty time! Faster for next time!');
            Choose:=0;
            Exit
          END
        END;
      IF NOT EmptyTime THEN Ch:=Readkey;
      SayXY(X,Y+I,White,Blue,Option[i]+#32);
      CASE Ch OF { Cai dat ma phim de chon cau tra loi }
        #0 : CASE Readkey OF
               #72 : IF i > 1 THEN Dec(i) ELSE I:=NumOpt;
               #80 : IF i < NumOpt THEN Inc(i) ELSE I:=1
             END;
        '1','A','a' : I:=1;
        '2','B','b' : I:=2;
        '3','C','c' : I:=3;
        '4','D','d' : I:=4;
        'Q','q' : EndProgram;
      END;
    UNTIL (Ch = #13) OR (EmptyTime);
    IF Ch = #13 THEN Choose:=i;
  END;

{---------------------------------------------}
{ Thu tuc doc de thi va bat dau cho thi       }
{ Vao : (Stt) so thu tu,                      }
{       (ExamQues) cau hoi,                   }
{       (Mark) so diem tung phan,             }
{       (Answer) cau tra loi,                 }
{       (MessOpt) thong bao cau hoi dung.     }
{ Ra  : Khong                                 }
{---------------------------------------------}
PROCEDURE Checking(Stt : Byte; ExamQues : ArrQues; VAR Mark : Byte;
                   VAR Answer : Str80; MessOpt : ArrQues);
VAR
  i,n,Choose,Tc : Byte;
  Ques, NumQues,
  Num, Mess, Answ : str80;
  ExamTest : ArrQues;
  Emptytime : Boolean;
BEGIN
  Tc:=0;
  SetTextColor(White,Blue);
  Clrscr;
  Str(Stt,NumQues);
  Ques:=Copy(ExamQues[1],1,79); { Doc cau hoi tu bo de thi }
  FOR I:=1 TO 4 DO
   BEGIN
     IF ExamQues[i+1][1]='*' THEN N:=i; { Kiem tra cau tra loi dung }
    ExamTest[i]:=RightAdd(ExamQues[i+1],70); { Chen khoang trong }
    ExamTest[i]:=Copy(ExamTest[i],2,Length(ExamTest[i])-1); { Lay cau hoi }
   END;
  Frame(28,3,52,5,1);
  SetTextColor(LightRed,Blue);
  WriteCtr(4,'Question Number : '+NumQues);
  SetTextColor(White,Blue);
  WriteCtr(7,Ques);
  Menu(6,10,ExamTest,4,Choose,EmptyTime,MessOpt); { Hien Menu cac cau hoi }
  SetTextColor(10,Blue);
  IF EmptyTime THEN { Bat dau cho thi }
    BEGIN
      Mark := 0;
      Str(N,Num);
      WriteSnd(19,'Correct answer is : '+Num+' ÄÄ '+ExamTest[n]);
      Mess:=' Not Enough Time !';
      Answ:='Empty'
    END
  ELSE
    IF Choose <> n THEN { Kiem tra chon lua va ra thong bao }
      BEGIN
        Mark := 0;
        WriteCtr(18,'Incorrect !');
        Str(N,Num);
        WriteSnd(19,'Correct answer is : '+Num+' ÄÄ'+ExamTest[n]);
        Mess:=' Incorrect ';
        Answ:=AllTrim(ExamTest[Choose])
      END
    ELSE
      BEGIN
        Mark := 1;
        WriteCtr(18,'Congratulation!'+#7);
        Mess:=' Correct ';
        Answ:=AllTrim(ExamTest[Choose]);
        Delay(200)
   END;
   Answer:='Question : '+NumQues+'#'+Ques+'# >>> Answer : '+Answ+
   '# >>> Solutions : '+AllTrim(ExamTest[n])+'# >>> Comment : '+Mess
END;

{-------------------------------------------------}
{ Thu tuc do cau hoi va hien len man hinh         }
{ Vao : (Name) ten thi sinh,                      }
{       (SerialNumber) so bao danh.               }
{ Ra  : Khong                                     }
{-------------------------------------------------}
PROCEDURE ReadExamQues(Name,SerialNumber : Str80);
VAR
  FileData, FileResult : Text;
  Senc, MessOpt : ArrQues;
  Aux, Mess, Comment, DoSenc,
  TotalMark, Aver, DataFile, ResultFile : Str80;
  Num, i, Mark, NumSenc,Total : Byte;
  Average : Real;
  Key     : Char;
  BEGIN
    {LoadFont('1');}
    SetBorderColor(47);
    DataFile := 'question.txt';{GetDisk+'TOPICS\CHECKINF\question.dat';}
    ResultFile := 'result.txt';{GetDisk+'TOPICS\CHECKINF\result.txt';}
    {$I-}
    Assign(FileData,DataFile);
    Reset(FileData);
    {$I+}
    IF IOResult <> 0 THEN
      BEGIN
        SetTextColor(Yellow,Blue);
        ClrScr;
        SetCurSorSize(TurnOff);
        SayXY(28,9,White,Blue,'THE PROGRAM TERMINATED !');
        WriteXY(13,11,'Cannot read from file Question.dat, you sure that the file');
        WriteXY(13,12,'is present in current directory. Press <ENTER> key to quit');
        WriteXY(13,13,'If you haven`t this file please contact with the author to');
        WriteXY(13,14,'copy it. Thank you for using this program. See you again !');
        REPEAT UNTIL KeyPressed;
        EndProgram;
      END;
    Assign(FileResult,ResultFile);
    Rewrite(FileResult);
    Writeln(FileResult,'**** Result of the test : ',Name,' . Serial number : ', SerialNumber, ' ****');
    Total:=0;
    Num:=0;
    WHILE NOT EOF(FileData) DO
      BEGIN
        i := 0;
        Inc(Num);
        REPEAT
          Readln(FileData,Aux);
          Inc(i);
          Senc[i]:=Aux
        UNTIL i = 5;
        IF Num = 1 THEN
          BEGIN
            MessOpt[1]:='0';
            MessOpt[2]:='0';
            MessOpt[3]:='0'
          END;
        Checking(Num,Senc,Mark,Comment,MessOpt);
        Total:=Total+Mark;
        Str(Num,DoSenc);
        Str(Total,TotalMark);
        Str(10*(Total/Num):0:1,Aver);
        MessOpt[1]:=DoSenc;
        MessOpt[2]:=TotalMark;
        MessOpt[3]:=Aver;
        {Writeln(FileResult,Comment)}
        {FOR i:=1 TO Length(Str) DO
         IF Str[i]='#' THEN Writeln
         ELSE Write(Str[i]);
         Writeln;
         Write(Replicate('*',78));}

     END;
     NumSenc:=Num;
     Average:=(Total/NumSenc)*10;
     IF (Average>=0) AND (Average<5) THEN Mess:=' Weak';
     IF (Average>=5) AND (Average<6.5) THEN Mess:=' Average';
     IF (Average>=6.5) AND (Average<8) THEN Mess:=' Enough';
     IF (Average>=8) AND (Average<9.5) THEN Mess:=' Good';
     IF (Average>=9.5) AND (Average<=10) THEN Mess:=' Very Good !';
     Writeln(FileResult,'Total : ',Average:0:1,' Marks . Comment :'+Mess);
     Close(FileData);
     Close(FileResult);
     SetCursorSize(Normal)
END;

{---------------------------------}
{ Thu tuc gioi thieu chuong trinh }
{ Vao : Khong                     }
{ Ra  : Khong                     }
{---------------------------------}
PROCEDURE Introduction;
VAR S1,S2,S3,
    S4,S5 : Str80;
    I : Integer;
BEGIN
  DoBlinking(TRUE);
  SetBorderColor(63);
  SetCursorSize(TurnOff);
  S1:='<<<<<< University Of Technology Ho Chi Minh City >>>>>> ';
  S2:='<<<<<< Major Of Informatic Technology >>>>>> ';
  S3:='Basic Program Informatic Checking ';
  S4:='The Part Of Review Theory , The Form Questions Of Testing ';
  S5:='Include 40 Questions To Concerned With Operation MS-DOS & Norton Commander';
  SetTextColor(Yellow,1);
  ClrScr;
  Frame(1,1,80,24,2);
  BoxShadow(10,3,70,5,$0F,$05,$07,$00,2);
  SayXY(20,4,Yellow,Magenta,St3);
  TextColor(LightCyan);
  WriteSndCtr(08,S1);
  WriteSndCtr(10,S2);
  WriteSndCtr(12,S3);
  WriteSndCtr(14,S4);
  WriteSndCtr(16,S5);
  Textcolor(Yellow+Blink);
  WriteCtr(20,'Press any key to continue ...');
  Runchar(St1);
  SetTextColor(10,1);
  WriteSnd(20,'This program is protected by : Nguyen Ngoc Van (21/10/2001)');
  REPEAT UNTIL KeyPressed;
  NextPage
END;

{------------------------------------------}
{ Thu tuc viet ket qua tu dia ra man hinh  }
{ Vao : (Str) dong tieu de                 }
{ Ra  : Khong                              }
{------------------------------------------}
PROCEDURE WriteResult(Str : Str80);
VAR i : Byte;
  BEGIN
    FOR i:=1 TO Length(Str) DO
      IF Str[i]='#' THEN Writeln
      ELSE Write(Str[i]);
    Writeln;
    Write(Replicate('*',78));
  END;

{---------------------------}
{ Thu tuc tong ket diem     }
{ Vao : Khong               }
{ Ra  : Khong               }
{---------------------------}
PROCEDURE Summary;
VAR
 F : Text;
 S : STRING;
 i, OldColor : Byte;
 Key : Char;
 BEGIN
   SetTextColor(10,1);
   Window(1,1,80,25);
   clrscr;
   BoxStr(1,1,80,24,1,' Table Of Contents ');
   Window(2,2,79,23);
   SetTextColor(Yellow,Blue);
   Clrscr;
   OldColor:=TextAttr;
   Assign(F,'Result.txt');
   Reset(F);
   I:=0;
   WHILE NOT EOF(F) DO
     BEGIN
       Inc(i);
       Readln(F,S);
       WriteResult(S);
       IF (I mod 3) = 0 THEN
         BEGIN
           REPEAT
             TextColor(Random(15)+1);
             WriteCtr(21,'Press any key to display or <ESC> to stop ...');
             Delay(100)
           UNTIL KeyPressed;
           IF KeyPressed THEN
             BEGIN
               Key:=Readkey;
               IF Key = #27 THEN Halt
             END;
           TextAttr:=OldColor;
           NextPage
         END;
     END;
     Close(F);
     Window(1,1,80,25);
     SetCursorSize(TurnOff);
     WriteCtr(15,'Thank you for using My Program. I hope that your knowledge');
     WriteCtr(16,'to become bester after you passed this examination.');
     WriteCtr(17,'Goodbye and see you again !');
     TextColor(LightCyan);
     WriteCtr(20,'Press any key to exit program ...');
     Runchar(St2)
  END;

{-----------------------------------------}
{ Kiem tra thoi gian su dung chuong trinh }
{ Vao : Khong                             }
{ Ra  : Khong                             }
{-----------------------------------------}
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
           SayXY(30, 10, $0F, $01,'SYSTEM ERROR!');
           SayXY(20, 12, $0C, $01,'Cannot reading from file register.dat');
           SayXY(20, 13, $0C, $01,'Please press ENTER key to exit program');
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
                ReleaseTopics;
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
                ReleaseTopics;
                EndProgram;
              END
        END
  END;

{--------------------------------}
{ Tien hanh dang ky chuong trinh }
{ Vao : Khong                    }
{ Ra  : Khong                    }
{--------------------------------}
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
           SetCursorSize($2020);
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
     SetCursorSize($0B0A);
     SetTextColor($00,$03);
     Readln(CurrName);
     IF OldName <> CurrName THEN
        BEGIN
           Sound(2000);
           SayXY(31,7,$0F,$03, Replicate(#32,30));
           SayXY(31,7,$00,$03,'Incorrect your name !');
           Delay(200);
           Nosound;
        END;
     GotoXY(31,9);
     SetCursorSize($0B0A);
     SetTextColor($00,$03);
     Readln(CurrNum);
     IF OldNum <> CurrNum THEN
        BEGIN
           Sound(2000);
           SayXY(31,9,$0F,$03, Replicate(#32,30));
           SayXY(31,9,$00,$03,'Incorrect your register!');
           Delay(200);
           Nosound;
        END;
     IF (OldName = CurrName) AND (OldNum = CurrNum) THEN
        BEGIN
          Sound(2000);
          SayXY(31,8,$0F,$03, Replicate(#32,30));
          SayXY(31,8,$00,$03,'Register is successfull!');
          Delay(200);
          Nosound;
          IsRegister := 1;
          Readln
        END
     ELSE
        BEGIN
          Sound(2000);
          SetCursorSize($2020);
          SayXY(31,8,$0F,$03, Replicate(#32,30));
          SayXY(31,8,$00,$03,'Register is not successfull!');
          Delay(200);
          Nosound;
          Readln
        END
   END;

{-----------------------}
{ Cap nhat file dang ky }
{ Vao : Khong           }
{ Ra  : Khong           }
{-----------------------}
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

{-----------------------------------}
{ Xu ly muc chon dang ky ban quyen  }
{ Vao : (Select) muc chon tuong ung }
{ Ra  : Khong                       }
{-----------------------------------}
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

{-----------------------------}
{ Hien menu dang ky ban quyen }
{ Vao : Khong                 }
{ Ra  : Khong                 }
{-----------------------------}
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

{----------------------------------------}
{ Ham hen thong bao dang ky chuong trinh }
{ Vao : Khong                            }
{ Ra  : Khong                            }
{----------------------------------------}
PROCEDURE Register;
VAR DiskLetter : Str40;
   BEGIN
     SetBorderColor(55);
     SetTextColor(Blue,White);
     SetCursorSize($2020);
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
     BoxShadow(3,2,77,23,$0F,$04,$08,$07,2);
     WriteVRAM(5,2,$4F,'[ ]');
     WriteVRAM(73,2,$4F,'[ ]');
     WriteChar(6,2,$4A,1,Chr(254));
     WriteChar(74,2,$4A,1,Chr(18));
     SayXY(34,2,$0A,$03,' Unregister ');
     SayXY(9,3,$0F,$04,'You have to register this copy in order to continue using it');
     SayXY(14,4,$0A,$04,'(The copy registered does not showing this message)');
     SayXY(5,5,$0F,$04,'If you already have the register number. Choose register command and');
     SayXY(5,6,$0F,$04,'type in the number into the number box and push register now button.');
     SayXY(14,7,$0B,$04,'Enter Your Name');
     SayXY(31,7,$0F,$03, Replicate(#32,30));
     SayXY(14,9,$0B,$04,'Register Number');
     SayXY(31,9,$0F,$03, Replicate(#32,30));
     SayXY(5,10,$0F,$04,'If you do not have the register number yet. Please send to me this');
     SayXY(5,11,$0F,$04,'product number JD0L-E3V6-4674 with the fund of $15 to the following');
     SayXY(5,12,$0F,$04,'address (You can use the normal postal services)');
     SayXY(14,13,$0A,$04,'NGUYEN NGOC VAN Sundling');
     SayXY(14,14,$0A,$04,'57A Precinct 1st District 4th NGUYEN KIEU Island');
     SayXY(14,15,$0A,$04,'Phone : 9300061 - Email : nguyenvan@yahoo.com');
     SayXY(5,16,$0F,$04,'When I received your product number and money. I will send back to you');
     SayXY(5,17,$0F,$04,'a register number of this program. By buying the software, you are');
     SayXY(5,18,$0F,$04,'helping me survive and continue to upgrade the software better!');
     SayXY(7,19,$0A,$04,'THANH YOU FOR YOUR HELPING BY REGISTERING AND USING TOPICS PROGRAM');
     ShowingMenuRegister;
   END;

{-------------------------------}
{      CHUONG TRINH CHINH       }
{ Nhiem vu : Thuc hien cac ket  }
{ noi de thuc hien chuong trinh }
{-------------------------------}
BEGIN
{ IF NOT CheckRegister THEN
    BEGIN
       CheckPeriod;
       Register;
    END;}
 CheckBreak:=False;               { Ngan cang viec dung chuong trinh      }
 Introduction;                    { Gioi thieu chuong trinh               }
 ReadName(Name,SerialNumber);     { Doc ten va so bao danh                }
 ReadExamQues(Name,SerialNumber); { Doc de thi va tien hanh kiem tra      }
 Summary;                         { Tom tat ket qua kiem tra              }
 EndProgram                       { Giai phong chuong trinh an toan       }
END.
{*************************************************************************}
{*********************KET THUC CHUONG TRINH NGUON*************************}
{*************************************************************************}
