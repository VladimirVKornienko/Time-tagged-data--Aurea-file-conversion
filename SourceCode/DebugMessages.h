#pragma once

//	*	*	*	*	*	*	*	*	*	*	*	//
//	*	Debug flags (uncomment to use):		*	//
//	*	*	*	*	*	*	*	*	*	*	*	//


#ifndef AureaProcessorPart2OverflowDebuggingMessages
//#define AureaProcessorPart2OverflowDebuggingMessages
// << uncomment to use debugging on first 10 records. //
#endif

#ifndef AureaReverseChannelAndTimeData
//#define AureaReverseChannelAndTimeData

// << uncomment to write time tag first, and only then the channel data. //
#endif


#ifndef AureaNoHeaderToResultFile
// #define AureaNoHeaderToResultFile
// << uncomment for DEBUG MODE (ONLY). //
#endif

#ifndef stage4_Write_Overflow_Flags_ToFile
// #define stage4_Write_Overflow_Flags_ToFile
// (un)comment for writing (skipping) overflow markers when de-coding back from PTU format to Aurea format.
// will be used in [stage3] also, if
//				<stage3_DEBUG_re_create_files_with_text_tag_time_values> is defined.
#endif

#ifndef stage3_DEBUG_re_create_files_with_text_tag_time_values
// #define stage3_DEBUG_re_create_files_with_text_tag_time_values

// uncomment to create 2 add. files with tags and times, as seen in stage 3.
// (Not from PTU file, as in Stage 4).

// this checks for the consistency of aux. files, generated suring the stage 1.
// it has been proof-checked, it is working finely.

// NB! works only for the last file in a batch (matter of simplicity, and enough for debugging).
#endif