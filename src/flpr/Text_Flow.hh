/*
   Copyright (c) 2019-2020, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*!
  \file Text_Flow.hh
*/

#ifndef FLPR_TEXT_FLOW_HH
#define FLPR_TEXT_FLOW_HH 1

#include "flpr/File_Info.hh"
#include "flpr/File_Line.hh"
#include "flpr/Line_Accum.hh"
#include "flpr/Safe_List.hh"
#include "flpr/Token_Text.hh"


/*
  - An input buffer of strings is converted to File_Lines.
  - Line_Groups group File_Lines as runs of non-program text, a single
    program text line, or a continued program text line, possibly with
    internal non-program text lines.
  - The main text of a program text Line_Group is concatenated with a 
    Line_Accum and parsed with flex to identify tokens.
  - The token extents are used to split the main text into a list of
    text segments: token text, interspersed with spaces and 
    token/line/statement breaks.  This list is called the flow.
  - Token_Text are created to hold the Syntax_Tag and references to the
    token text segments in the flow.  
  - Token_Text are organized into statements.

  - Line_Group contains a list of File_Line iterators that contain program text.  
  - We want expandable lines (`include` or `#include`

*/



#endif
