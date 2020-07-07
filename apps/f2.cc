#include <array>
#include <flpr/Safe_List.hh>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace Text_Flow {

enum struct Type {
  COMMAND,
  FIXED_CONTROL,
  FREE_CONTROL,
  PINNED,
  SEGMENT,
  SPACES,
  TAB,
  UNKNOWN
};

struct Command;
struct Fixed_Control;
struct Free_Control;
struct Pinned;
struct Segment;
struct Spaces;
struct Tab;

struct Visitor {
  virtual void visit(Command &) {}
  virtual void visit(Fixed_Control &) {}
  virtual void visit(Free_Control &) {}
  virtual void visit(Pinned &) {}
  virtual void visit(Segment &) {}
  virtual void visit(Spaces &) {}
  virtual void visit(Tab &) {}
};

struct Const_Visitor {
  virtual void visit(Command const &) {}
  virtual void visit(Fixed_Control const &) {}
  virtual void visit(Free_Control const &) {}
  virtual void visit(Pinned const &) {}
  virtual void visit(Segment const &) {}
  virtual void visit(Spaces const &) {}
  virtual void visit(Tab const &) {}
};

struct Abstract_Entity {
  virtual void accept(Visitor &) = 0;
  virtual void accept(Const_Visitor &) = 0;
  virtual ~Abstract_Entity() = default;
  virtual std::ostream &print(std::ostream &os) const = 0;
};

template <typename Specific_Entity> struct Entity : public Abstract_Entity {
  explicit Entity(Type t) : type{t} {}
  Type type = Type::UNKNOWN;
  void accept(Visitor &v) override {
    v.visit(*(static_cast<Specific_Entity *>(this)));
  }
  void accept(Const_Visitor &v) override {
    v.visit(*(static_cast<Specific_Entity const *>(this)));
  }
  virtual std::ostream &print(std::ostream &os) const = 0;
};

struct Command : public Entity<Command> {
  enum struct CMD { CONTINUATION, EOS, NEW_LINE, NEW_STMT, TOKBRK, UNKNOWN };
  Command() : Entity{Type::COMMAND}, cmd{CMD::UNKNOWN} {}
  explicit Command(CMD c) : Entity{Type::COMMAND}, cmd{c} {}
  std::ostream &print(std::ostream &os) const override {
    switch (cmd) {
    case CMD::CONTINUATION:
      os << "L&";
      break;
    case CMD::EOS:
      os << "EOS";
      break;
    case CMD::NEW_LINE:
      os << "NL";
      break;
    case CMD::NEW_STMT:
      os << "NS";
      break;
    case CMD::TOKBRK:
      os << "TOKBRK";
      break;
    case CMD::UNKNOWN:
      os << "?";
      break;
    }
    return os;
  }

public:
  CMD cmd;
};

//! A fixed-format control block
/*! The first six columns of a fixed-format line.  This is required for every
    fixed-format line.  The default constructor makes an empty control block */
struct Fixed_Control : public Entity<Fixed_Control> {
  Fixed_Control() : Entity{Type::FIXED_CONTROL}, text{} {}
  explicit Fixed_Control(std::string const &txt)
      : Entity{Type::FIXED_CONTROL}, text{txt} {
    assert(text.size() == 6);
  }
  explicit Fixed_Control(std::string &&txt)
      : Entity{Type::FIXED_CONTROL}, text{std::move(txt)} {
    assert(text.size() == 6);
  }
  Fixed_Control(unsigned const label, bool left_justified)
      : Entity{Type::FIXED_CONTROL}, text{"      "} {
    assert(label > 0 && label < 99999);
    std::string ls = std::to_string(label);
    if (left_justified) {
      text.replace(0, ls.size(), ls);
    } else {
      text.replace(5 - ls.size(), ls.size(), ls);
    }
  }
  explicit Fixed_Control(char const continuation)
      : Entity{Type::FIXED_CONTROL}, text{"      "} {
    text[5] = continuation;
  }
  Fixed_Control(std::string const &ctext, char const comment)
      : Entity{Type::FIXED_CONTROL}, text{} {
    text.push_back(comment);
    text.append(ctext);
  }
  std::ostream &print(std::ostream &os) const override {
    return os << '|' << text << '|';
  }
  void set_continuation(char const c) {
    if (text.empty())
      text.append(6, ' ');
    text[5] = c;
  }

public:
  std::string text;
};

//! A free-format "control block"
/*! A label or leading continuation character, plus all leading spaces, for a
    free-format line.  You don't need to have one of these if it is empty
    anyway.  */
struct Free_Control : public Entity<Free_Control> {
  Free_Control() : Entity{Type::FREE_CONTROL}, text{} {}
  explicit Free_Control(std::string const &txt)
      : Entity{Type::FREE_CONTROL}, text{txt} {}
  explicit Free_Control(std::string &&txt)
      : Entity{Type::FREE_CONTROL}, text{std::move(txt)} {}
  std::ostream &print(std::ostream &os) const override {
    return os << '"' << text << '"';
  }

public:
  std::string text;
};

/*! A chunk of text pinned to a certain column.  This is for '!' comments */
struct Pinned : public Entity<Pinned> {
  Pinned() : Entity{Type::PINNED}, text{}, col{0} {}
  Pinned(std::string const &txt, unsigned const c)
      : Entity{Type::PINNED}, text{txt}, col{c} {}
  explicit Pinned(std::string &&txt, unsigned const c)
      : Entity{Type::PINNED}, text{std::move(txt)}, col{c} {}
  std::ostream &print(std::ostream &os) const override {
    return os << '"' << text << '"';
  }

public:
  std::string text;
  unsigned col;
};

/*! A chunk of text.  Segments are assembled together to create Tokens */
struct Segment : public Entity<Segment> {
  Segment() : Entity{Type::SEGMENT}, text{} {}
  explicit Segment(std::string const &txt) : Entity{Type::SEGMENT}, text{txt} {}
  explicit Segment(std::string &&txt)
      : Entity{Type::SEGMENT}, text{std::move(txt)} {}
  std::ostream &print(std::ostream &os) const override {
    return os << '"' << text << '"';
  }

public:
  std::string text;
};

struct Spaces : public Entity<Spaces> {
  Spaces() : Entity{Type::SPACES}, n{0} {}
  explicit Spaces(unsigned const ns) : Entity{Type::SPACES}, n{ns} {}
  std::ostream &print(std::ostream &os) const override {
    return os << 'S' << n;
  }

public:
  unsigned n; //>>! number of spaces this represents
};

struct Tab : public Entity<Tab> {
  //! The column where main program text starts
  /*! This is used for indenting the main program text.  First, write any
    fixed_control block (label or continuation) data, then the Tab.  If the
    output line does not already extend to `col`, Tab will append enough spaces
    to make it so.  If a label immediately preceeds the Tab col, add an extra
    space. */
  Tab() : Entity{Type::TAB}, col{0} {}
  explicit Tab(unsigned const c) : Entity{Type::TAB}, col{c} {}
  std::ostream &print(std::ostream &os) const override {
    return os << 'T' << col;
  }

public:
  unsigned col;
};

} // namespace Text_Flow

using TF_List = FLPR::Safe_List<std::unique_ptr<Text_Flow::Abstract_Entity>>;

struct Render : public Text_Flow::Const_Visitor {
  void visit(Text_Flow::Command const &e) override {
    switch (e.cmd) {
    case Text_Flow::Command::CMD::CONTINUATION:
      curr_line.push_back('&');
      break;
    case Text_Flow::Command::CMD::NEW_LINE:
      curr_line.push_back('\n');
      result += curr_line;
      curr_line.clear();
      break;
    case Text_Flow::Command::CMD::NEW_STMT:
      curr_line.push_back(';');
      break;
    default:
      break;
    }
  }
  void visit(Text_Flow::Fixed_Control const &e) override {
    fixed_format = true;
    curr_line = e.text;
  }
  void visit(Text_Flow::Free_Control const &e) override {
    fixed_format = false;
    curr_line = e.text;
  }
  void visit(Text_Flow::Pinned const &e) override {
    pad_to_col(e.col);
    curr_line += e.text;
  }
  void visit(Text_Flow::Segment const &e) override { curr_line += e.text; }
  void visit(Text_Flow::Spaces const &e) override {
    curr_line.append(e.n, ' ');
  }
  void visit(Text_Flow::Tab const &e) override {
    pad_to_col(e.col);
    if (!fixed_format && std::isdigit(curr_line.back())) {
      curr_line.push_back(' ');
    }
  }
  void pad_to_col(unsigned const col) {
    if (curr_line.size() < col - 1) {
      curr_line.append((col - 1) - curr_line.size(), ' ');
    }
  }

public:
  bool fixed_format{false};
  std::string curr_line;
  std::string result;
};

int main(int argc, char const **argv) {
  TF_List text_flow;
  text_flow.emplace_back(std::make_unique<Text_Flow::Fixed_Control>(
      "     this is really important", '*'));
  text_flow.emplace_back(
      std::make_unique<Text_Flow::Command>(Text_Flow::Command::CMD::NEW_LINE));

  text_flow.emplace_back(std::make_unique<Text_Flow::Fixed_Control>(99, false));
  text_flow.emplace_back(std::make_unique<Text_Flow::Tab>(7));
  text_flow.emplace_back(std::make_unique<Text_Flow::Segment>("call"));
  text_flow.emplace_back(std::make_unique<Text_Flow::Spaces>(1));
  text_flow.emplace_back(std::make_unique<Text_Flow::Segment>("foo"));
  text_flow.emplace_back(std::make_unique<Text_Flow::Segment>("("));
  text_flow.emplace_back(std::make_unique<Text_Flow::Spaces>(2));
  text_flow.emplace_back(std::make_unique<Text_Flow::Spaces>(2));
  text_flow.emplace_back(std::make_unique<Text_Flow::Pinned>("! comment", 40));
  text_flow.emplace_back(
      std::make_unique<Text_Flow::Command>(Text_Flow::Command::CMD::NEW_LINE));
  text_flow.emplace_back(std::make_unique<Text_Flow::Fixed_Control>('&'));
  text_flow.emplace_back(std::make_unique<Text_Flow::Tab>(9));
  text_flow.emplace_back(std::make_unique<Text_Flow::Segment>("a"));
  text_flow.emplace_back(std::make_unique<Text_Flow::Segment>(","));
  text_flow.emplace_back(std::make_unique<Text_Flow::Segment>("b"));
  text_flow.emplace_back(std::make_unique<Text_Flow::Segment>(")"));
  text_flow.emplace_back(
      std::make_unique<Text_Flow::Command>(Text_Flow::Command::CMD::EOS));
  text_flow.emplace_back(
      std::make_unique<Text_Flow::Command>(Text_Flow::Command::CMD::NEW_LINE));

  Render render;
  for (auto const &ep : text_flow) {
    ep->accept(render);
    std::cout << ' ';
    ep->print(std::cout);
  }
  std::cout << '\n';
  std::cout << "         1         2         3         4         5         6"
               "         7         8\n"
               "123456789012345678901234567890123456789012345678901234567890"
               "12345678901234567890\n";
  std::cout << render.result;
  return 0;
}
