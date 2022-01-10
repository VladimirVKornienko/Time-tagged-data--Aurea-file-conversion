#pragma once

//	*	*	*	*	*	*	*	*	*	*	*	//
//	*	Debug flags (uncomment to use):		*	//
//	*	*	*	*	*	*	*	*	*	*	*	//

#ifndef use_back_conversion_of_stage_4
        // IMP //
// #define use_back_conversion_of_stage_4
// uncomment to use back-conversion from PTU format to AUREA intrinsic format.
#endif


#ifndef use_log_file_output
#define use_log_file_output
// uncomment to write log to file
#endif


#ifndef stage4_Write_Overflow_Flags_ToFile
        // IMP //
// #define stage4_Write_Overflow_Flags_ToFile

// (un)comment for writing (skipping) overflow markers when de-coding back from PTU format to Aurea format.

// will be used in [stage3] also, if
//				<stage3_DEBUG_re_create_files_with_text_tag_time_values> is defined.

// 06.01.2022: works finely. //

#endif

#ifndef stage3_DEBUG_re_create_files_with_text_tag_time_values
        // IMP //
// #define stage3_DEBUG_re_create_files_with_text_tag_time_values

// uncomment to create 2 add. files with tags and times, as seen in stage 3.
// (Not from PTU file, as in Stage 4).

// this checks for the consistency of aux. files, generated suring the stage 1.
// it has been proof-checked, it is working finely.

// NB! works only for the last file in a batch (matter of simplicity, and enough for debugging).

// 06.01.2022: works finely. //

#endif


#ifndef keep_temporary_files
// #define keep_temporary_files
// when commented, the processor will delete temporary files (header + binary ch1, ch2)
#endif

#ifndef AureaProcessorPart2OverflowDebuggingMessages
//#define AureaProcessorPart2OverflowDebuggingMessages
// << uncomment to use debugging on first 10 records. //

// 06.01.2022: obsolete. //

#endif

#ifndef AureaReverseChannelAndTimeData
//#define AureaReverseChannelAndTimeData

// << uncomment to write time tag first, and only then the channel data. //

// 06.01.2022: not checked in the last version, may be faulty.. //

#endif


#ifndef AureaNoHeaderToResultFile
// #define AureaNoHeaderToResultFile
// << uncomment for DEBUG MODE (ONLY). //

// 06.01.2022: o.k., not checked. //

#endif