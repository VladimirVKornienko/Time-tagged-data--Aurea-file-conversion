# Time-tagged-data--Aurea-file-conversion
Converting time-tagged data from Aurea Technology (c) file format to *.ptu.

Convert four-column data to two column with overflow markers.
C/C++ based.
Target system: Windows 7 x64.
Should be platform-independent, but tested on Win7 x64 and Win10 x64 only.


NB!    >>>>>>

Found a bug:: subtracting two uint64 values (before multiplying them by double)

auxTimePS = ((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime);

<<<<<    !NB

Current tasks:

(-1) Create reverse wrapper (ptu -> Aurea formats)

(0) Check float-point operations when introducing an overflow event/marker;
maybe, precision is lost in (numerous) type castings

(1) [later] Add buffer for read/write operations on files [stage1, Stage3]

(2) [later] Parse the header of the AUREA file (currently, only the necessary info is extracted)
// >> check C-libraries of readPTU module for methods << //

(3) Check that this line is removed (bad behaviour on large files):
std::ios_base::sync_with_stdio(true);

(4) Check unwrapping complex bit structures for LSB / MSB conflicts.
See also:
https://stackoverflow.com/questions/24698096/struct-bit-packing-and-lsb-msb-ambiguity-c

(5) small tasks:
(5.i) check the order of the arguments in readTTTR records function:
>>> / readTTTR records / 
static inline void load_buffer(uint32_t *pbuffer, FILE *fhandle)
{
    if(fread(pbuffer, RECORD_CHUNK, sizeof(uint32_t), fhandle)==0) {
<<<

(5.ii) check type conversion in read/write operations:
reinterpret_cast<char*>(&d)

(5.iii) check if needs to be applied somewhere in header reading
>>>>
_fseeki64()

int fseek(
   FILE *stream,
   long offset,
   int origin
);
int _fseeki64(
   FILE *stream,
   __int64 offset,
   int origin
);
<<<<

(5.iv) Change measurement time calculation to a precise one (add Time, not only Tag):
"AureaPreProcessingToHH_T2.cpp" -> at "// KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK"


(5.v) Stage2 processor:: make changes from Stage3.
Most importantly, 'while' loop for overflow events.


(5.vi) implement normal type casting between {picoseconds; overflows} and
					{tags; times}.

(5.vii) in stage3, changed <convertedCH2timePS> from uint32_t to double.
Conversion is now performed at the very last step!

(5.viii) [stage3] ToDo: replace "ch2 left" with fscanfSTATUS == EOF (-1).

[(End of ReadMe file.)]
