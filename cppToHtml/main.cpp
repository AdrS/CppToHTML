#include <string>
#include <iostream>
#include <cstdio>
enum TokenType{ PREPROCESSOR, COMMENT, KEYWORD, IDENTIFIER, CONSTANT, STRING, SYMBOLS };
std::string tokenTypeToText(TokenType t) {
	switch (t) {
	case PREPROCESSOR: return "PREPROCESSOR";
	case COMMENT: return "COMMENT";
	case STRING: return "STRING";
	case CONSTANT: return "CONSTANT";
	case SYMBOLS: return "SYMBOLS";
	case KEYWORD: return "KEYWORD";
	case IDENTIFIER: return "IDENTIFIER";
	default: return "bs";
	}
}
struct Token {
	TokenType type;
	std::string contents;
	Token(std::string s, TokenType t) :contents(s) {
		type = t;
	}
	Token(const char* s, TokenType t) {
		contents = std::string(s);
		type = t;
	}
	virtual ~Token() {}
};
bool isWhiteSpace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\f';
}
bool isDecimalDigit(char c) {
	return c >= '0' && c <= '9';
}
bool isOctalDigit(char c) {
	return c >= '0' && c < '8';
}
bool isHexDigit(char c) {
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}
void skipWhiteSpace(const std::string& s, unsigned int& pos) {
	while (isWhiteSpace(s[pos]) && pos < s.length())pos++;
}
bool isSymbol(char c) {
	const char* s = "~!%^&*()-+={[}]|:;,<.>/?";
	for (unsigned int i = 0; i < 24; i++)
	if (c == s[i])return true;
	return false;
}
bool isValidNameCharacter(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
}
bool isKeyword(const std::string& s) {
	static const char* keywords[] = { "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char16_t", "char32_t", "class", "compl", "const", "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "final", "float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "override", "private", "protected", "public", "register", "reinterpret_cast", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq" };
	int min = 0, max = 85, guess, comp;
	while (max >= min) {
		guess = (min + max) / 2;
		comp = s.compare(keywords[guess]);
		if (comp == 0) {
			return true;
		}
		else if (comp > 0) {
			min = guess + 1;
		}
		else {
			max = guess - 1;
		}
	}
	return false;
}
/*
add support for multi line preprocessor derectives
add support for decimals constants ex: 1.09E-5
*/
Token getNextToken(const std::string& s, unsigned int& pos) {
	std::string temp = "";
	unsigned int start = pos;
	while (pos < s.length()) {
		if (s[pos] == '/') {
			if (pos + 1 < s.length() && s[pos + 1] == '/' || s[pos + 1] == '*') {	//comment
				pos += 2;
				if (s[pos - 1] == '/') {	//single line comment (get everythinig up to end of line)
					while (pos < s.length() && s[pos] != '\n') { pos++; }
					return Token(s.substr(start, pos - start), COMMENT);
				}
				else {		//multiline comment	(get everything until closing comment, or end of file)
					while (pos < s.length() && !(pos > start + 2 && s[pos - 2] == '*' && s[pos - 1] == '/')) { pos++; }
					return Token(s.substr(start, pos - start), COMMENT);
				}
			}
			else {		//operator	(must be the '/' operator
				pos++;
				return Token("/", SYMBOLS);
			}
		}
		else if (s[pos] == '#') {	//preprocessor directive (get everything until end of line, end of file, or start of a comment
			while (pos < s.length() && s[pos] != '\n' && !(pos + 1 < s.length() && s[pos] == '/' && (s[pos + 1] == '*' || s[pos + 1] == '/'))) { pos++; }
			return Token(s.substr(start, pos - start), PREPROCESSOR);
		}
		else if (s[pos] == '"') {	//string literal (get everything until closing quoteation mark or end of file, ignore escaped quotes)
			while (pos < s.length() && !(s[pos + 1] == '"' && s[pos] != '\\')) { pos++; }
			pos += 2;
			return Token(s.substr(start, pos - start), STRING);
		}
		else if (isDecimalDigit(s[pos])) {	//numerical constant (get until end of number)
			if (s[pos] == '0' && pos + 2 < s.length()) {
				if ((s[pos + 1] == 'o' || s[pos + 1] == 'O') && isOctalDigit(s[pos + 2])) {	//octal digit
					pos += 3;
					while (pos < s.length() && isOctalDigit(s[pos])) { pos++; }
				}
				else if ((s[pos + 1] == 'x' || s[pos + 1] == 'X') && isHexDigit(s[pos + 2])) {	//hex digit
					pos += 3;
					while (pos < s.length() && isHexDigit(s[pos])) { pos++; }
				}
				else {
					while (pos < s.length() && isDecimalDigit(s[pos])) { pos++; }
				}
			}
			else {
				while (pos < s.length() && isDecimalDigit(s[pos])) { pos++; }
			}
			return Token(s.substr(start, pos - start), CONSTANT);
		}
		else if (isSymbol(s[pos])) {	//get all symbols (stop at first non symbol or comment
			pos++;
			while (pos < s.length() && isSymbol(s[pos]) && !(s[pos] == '/' && pos + 1 < s.length() && (s[pos + 1] == '/' || s[pos + 1] == '*'))) { pos++; }
			return Token(s.substr(start, pos - start), SYMBOLS);
		}
		else if (isValidNameCharacter(s[pos]) && !isDecimalDigit(s[pos])) {	//keyword, or identifier
			pos++;
			while (pos < s.length() && isValidNameCharacter(s[pos])) { pos++; }
			temp = s.substr(start, pos - start);
			if (isKeyword(temp)) {
				return Token(temp, KEYWORD);
			}
			else {
				return Token(temp, IDENTIFIER);
			}
		}
		else {
			while (pos < s.length() && s[pos] != '/' && s[pos] != '"' && !isValidNameCharacter(s[pos]) && !isSymbol(s[pos])) { pos++; }
			return Token("//not writen yet", COMMENT);
		}
	}
	return Token("//end", COMMENT);
}
void htmlPrettyPrint(const std::string& s) {
	unsigned int pos = 0, start;
	std::string ret = "<!DOCTYPE html>\n<html>\n<head>\n	<style>\n	pre.CODE { background-color:#000000;}\n	span.PREPROCESSOR { color:#DDDDDD; }\n	span.COMMENT { color:#33FF33; }\n	span.KEYWORD { color:#FF33FF; }\n	span.IDENTIFIER { color:#FFFFFF; }\n	span.CONSTANT { color:#FF9933; }\n	span.STRING { color:#EE8726; }\n	span.SYMBOLS { color:#D3D3AF; }\n	</style>\n</head>\n<body>\n<pre class=\"CODE\">\n";
	while (pos < s.length()) {
		start = pos;
		skipWhiteSpace(s, pos);		//skip initial whitespace
		ret += s.substr(start, pos - start);
		Token t = getNextToken(s, pos);
		switch (t.type) {
		case PREPROCESSOR:
			ret += ("<span class=\"PREPROCESSOR\">" + t.contents + "</span>");
			break;
		case COMMENT:
			ret += ("<span class=\"COMMENT\">" + t.contents + "</span>");
			break;
		case STRING:
			ret += ("<span class=\"STRING\">" + t.contents + "</span>");
			break;
		case CONSTANT:
			ret += ("<span class=\"CONSTANT\">" + t.contents + "</span>");
			break;
		case SYMBOLS:
			ret += ("<span class=\"SYMBOLS\">" + t.contents + "</span>");
			break;
		case KEYWORD:
			ret += ("<span class=\"KEYWORD\">" + t.contents + "</span>");
			break;
		case IDENTIFIER:
			ret += ("<span class=\"IDENTIFIER\">" + t.contents + "</span>");
			break;
		default:;
		}
	}
	ret += "</pre>\n</body>\n</html>";
	std::cout << ret;
}
int main(int argc, const char** argv) {
	/*if (argc != 2) {
		std::cout << "usage: main src.cpp" << std::endl;
		return 0;
	}*/
	FILE* f = fopen("G:\\cppToHtml\\cppPrettyHtml.cpp", "r");
	if (!f)return 0;
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* data = new char[size + 1];
	fread(data, 1, size, f);
	fclose(f);
	data[size] = '\0';
	htmlPrettyPrint(std::string(data));
	return 0;
}