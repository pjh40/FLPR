/*
   Copyright (c) 2019-2020, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*!
  \file parse_files.cc

  This executable just runs the FLPR parser on a list of files.
*/

#include "flpr/Logical_File.hh"
#include "flpr/Prgm_Parsers.hh"
#include "flpr/Prgm_Tree.hh"
#include <fstream>
#include <iostream>
#include <unistd.h>

using Parse = FLPR::Prgm::Parsers<FLPR::Prgm::Prgm_Node_Data>;
using Parse_Tree = Parse::Prgm_Tree;

struct File {
  FLPR::Logical_File logical_file;
  Parse_Tree parse_tree;
};

bool read_file(std::string const &filename, std::vector<File> &files,
               bool const col72);
bool parse_cmd_line(std::vector<std::string> &filenames, int argc,
                    char *const argv[], bool &col72);

int main(int argc, char *const argv[]) {
  std::vector<std::string> filenames;
  bool col72{false};

  if (!parse_cmd_line(filenames, argc, argv, col72)) {
    std::cerr << "Usage: parse_files [-c] {-f <filename> | <filename>+}\n";
    std::cerr << "\t-c\t\tenforce 72-column limit in fixed format\n";
    std::cerr << "\t-f\t\tprovide a list of files to process\n";
    std::cerr << "exiting on error." << std::endl;
    return 1;
  }

  std::vector<File> files;
  for (auto const &f : filenames) {
    read_file(f, files, col72);
  }
  std::cout << "done." << std::endl;
  return 0;
}

bool read_file(std::string const &filename, std::vector<File> &files,
               bool const col72) {
  File f;
  std::cout << "Processing: '" << filename << "'"
            << "\n\tscanning..." << std::endl;
  if (!f.logical_file.read_and_scan(filename, (col72) ? 72 : 0)) {
    std::cout << "\tread/scan FAILED" << std::endl;
    return false;
  }
  std::cout << "\tscan created " << f.logical_file.lines.size()
            << " logical lines from " << f.logical_file.num_input_lines
            << " input text lines." << std::endl;
  std::cout << "\tparsing..." << std::endl;
  f.logical_file.make_stmts();
  Parse::State state(f.logical_file.ll_stmts);
  auto result{Parse::program(state)};
  if (!result.match) {
    std::cout << "\tparsing FAILED" << std::endl;
    return false;
  }

  auto c{result.parse_tree.ccursor()};
  std::cout << "\troot rule \"" << *c << "\" has " << c.node().num_branches()
            << " branches. " << '\n';

  f.parse_tree.swap(result.parse_tree);

  return true;
}

bool file_list_from_file(std::vector<std::string> &filenames,
                         char const *file_list_name) {
  std::ifstream is(file_list_name);
  if (!is) {
    std::cerr << "Unable to open file-list \"" << file_list_name << "\""
              << std::endl;
    return false;
  }
  const size_t orig_size = filenames.size();
  for (std::string fname; std::getline(is, fname);) {
    filenames.emplace_back(std::string{});
    filenames.back().swap(fname);
  }
  is.close();
  return filenames.size() > orig_size;
}

bool parse_cmd_line(std::vector<std::string> &filenames, int argc,
                    char *const argv[], bool &col72) {
  int ch;
  bool has_filelist{false};
  col72 = false;

  while ((ch = getopt(argc, argv, "cf:")) != -1) {
    switch (ch) {
    case 'c':
      col72 = true;
      break;
    case 'f':
      if (!file_list_from_file(filenames, optarg))
        return false;
      has_filelist = true;
      break;
    default:
      std::cerr << "unknown option" << std::endl;
      return false;
    }
  }
  argc -= optind;
  argv += optind;
  if (!has_filelist && argc == 0) {
    std::cerr << "need to specify at least one input file." << std::endl;
    return false;
  }
  for (int i = 0; i < argc; ++i) {
    filenames.emplace_back(std::string{argv[i]});
  }
  return true;
};
