#include "shader-parser.hpp"

#include <cctype>
#include <initializer_list>

namespace {
    struct Token {
        enum TokenType {
            TYPE_IDENTIFIER,
            TYPE_NUMERIC,
            TYPE_OPERATOR,

            TYPE_LAYOUT,
            TYPE_IN,
            TYPE_OUT,
            TYPE_UNIFORM,
            TYPE_BUFFER,
            TYPE_MEMORY_QUALIFIER,

            TYPE_OPEN_PAREN,
            TYPE_CLOSE_PAREN,
            TYPE_POUND_SIGN,
            TYPE_EQUAL_SIGN,
            TYPE_COMMA,
            TYPE_SEMI_COLON,
            TYPE_OPEN_CURLY,
            TYPE_CLOSE_CURLY,
            TYPE_OPEN_SQUARE,
            TYPE_CLOSE_SQUARE,

            TYPE_INVALID
        };

        TokenType type = TYPE_INVALID;
        String data;
        uint32 line;
    };

    void tokenizeShaderSource(std::istream& fileStream, ArrayList<Token>& tokens);

    bool consumeLayout(ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            ArrayList<ShaderInfo::Layout>& layoutInfo);
        
    bool consumeLayoutOptions(ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            ShaderInfo::Layout& li);
    bool consumeLayoutQualifiers(ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            ShaderInfo::Layout& li);
    bool consumeLayoutVariables(ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            ShaderInfo::Layout& li);

    bool expect(const ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            Token::TokenType type);
    bool expect(const ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            std::initializer_list<Token::TokenType> types);

    const char* stringifyTokenType(enum Token::TokenType type);
};

bool ShaderInfo::parse(std::istream& shaderData) {
    ArrayList<Token> tokens;
    ::tokenizeShaderSource(shaderData, tokens);

    for (auto it = tokens.begin(), end = tokens.end(); it != end; ++it) {
		if (it->type == Token::TYPE_LAYOUT) {
			if (!::consumeLayout(it, end, layoutInfo)) {
				return false;
			}
		}
	}

	return true;
}

ArrayList<ShaderInfo::Layout>& ShaderInfo::getLayoutInfo() {
    return layoutInfo;
}

const ArrayList<ShaderInfo::Layout>& ShaderInfo::getLayoutInfo() const {
    return layoutInfo;
}

const char* ShaderInfo::stringifyLayoutType(enum ShaderInfo::LayoutType type) {
	switch (type) {
		case ShaderInfo::LayoutType::UNIFORM_BUFFER:
			return "Uniform Buffer";
		case ShaderInfo::LayoutType::SHADER_STORAGE_BUFFER:
			return "Shader Storage Buffer";
		case ShaderInfo::LayoutType::ATTRIB_IN:
			return "Attribute In";
		case ShaderInfo::LayoutType::ATTRIB_OUT:
			return "Attribute Out";
		case ShaderInfo::LayoutType::UNIFORM:
			return "Uniform";
		default:
			return "Invaild Type";
	}
}

namespace {
    void tokenizeShaderSource(std::istream& fileStream, ArrayList<Token>& tokens) {
        char c;

        StringStream ss;

        uint32 line = 1;

        while (fileStream.good()) {
            c = fileStream.peek();

            if (std::isalpha(c)) {
                while (fileStream.good()) {
                    if (std::isalnum(c) || c == '_') {
                        ss << c;
                    }
                    else {
                        break;
                    }

                    fileStream.get();
                    c = fileStream.peek();
                }

                Token::TokenType type;
                String str = ss.str();

                // TODO: coherent, volatile, restrict (https://www.khronos.org/opengl/wiki/Image_Load_Store) as memory qualifiers
                // https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)#Memory_qualifiers

                if (str.compare("layout") == 0) {
                    type = Token::TYPE_LAYOUT;
                }
                else if (str.compare("in") == 0) {
                    type = Token::TYPE_IN;
                }
                else if (str.compare("out") == 0) {
                    type = Token::TYPE_OUT;
                }
                else if (str.compare("uniform") == 0) {
                    type = Token::TYPE_UNIFORM;
                }
                else if (str.compare("buffer") == 0) {
                    type = Token::TYPE_BUFFER;
                }
                else if (str.compare("readonly") == 0 || str.compare("writeonly") == 0) {
                    type = Token::TYPE_MEMORY_QUALIFIER;
                } 
                else {
                    type = Token::TYPE_IDENTIFIER;
                }

                tokens.push_back({type, str, line});
                ss.str("");
            }
            else if (std::isdigit(c)) {
                while (fileStream.good()) {
                    if (std::isdigit(c) || c == 'x' || c == 'X' || c == 'f' || c == '.') {
                        ss << c;
                    }
                    else {
                        break;
                    }

                    fileStream.get();
                    c = fileStream.peek();
                }

                tokens.push_back({Token::TYPE_NUMERIC, ss.str(), line});
                ss.str("");
            }
            else if (std::isspace(c)) {
                while (fileStream.good()) {
                    if (c == '\n') {
                        ++line;
                    }
                    else if (!std::isspace(c)) {
                        break;
                    }

                    fileStream.get();
                    c = fileStream.peek();
                }
            }
            else {
                fileStream.get();

                Token::TokenType type;

                switch (c) {
                    case '(':
                        type = Token::TYPE_OPEN_PAREN;
                        break;
                    case ')':
                        type = Token::TYPE_CLOSE_PAREN;
                        break;
                    case '#':
                        type = Token::TYPE_POUND_SIGN;
                        break;
                    case '=':
                        type = Token::TYPE_EQUAL_SIGN;
                        break;
                    case ',':
                        type = Token::TYPE_COMMA;
                        break;
                    case ';':
                        type = Token::TYPE_SEMI_COLON;
                        break;
                    case '{':
                        type = Token::TYPE_OPEN_CURLY;
                        break;
                    case '}':
                        type = Token::TYPE_CLOSE_CURLY;
                        break;
                    case '[':
                        type = Token::TYPE_OPEN_SQUARE;
                        break;
                    case ']':
                        type = Token::TYPE_CLOSE_SQUARE;
                        break;
                    default:
                        type = Token::TYPE_OPERATOR;
                }

                tokens.push_back({type, String(&c, 1), line});
            }
        }

        // rewind the stream, not strictly necessary
        //fileStream.clear();
        //fileStream.seekg(0);
    }

	bool consumeLayout(ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
			ArrayList<ShaderInfo::Layout>& layoutInfo) {
		if (!::expect(++it, end, Token::TYPE_OPEN_PAREN)) {
			return false;
		}

		ShaderInfo::Layout li;

		if (!::consumeLayoutOptions(it, end, li)) {
			return false;
		}

		if (!::consumeLayoutQualifiers(it, end, li)) {
			return false;
		}

		if (li.type == ShaderInfo::LayoutType::UNIFORM_BUFFER
				|| li.type == ShaderInfo::LayoutType::SHADER_STORAGE_BUFFER) {
			if (!::consumeLayoutVariables(it, end, li)) {
				return false;
			}
		}

		layoutInfo.push_back(li);

		return true;
	}

    bool consumeLayoutOptions(ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            ShaderInfo::Layout& li) {
        bool parsing = true;

        while (parsing) {
            if (!::expect(++it, end, Token::TYPE_IDENTIFIER)) {
                return false;
            }

            String ident = it->data;

            if (!::expect(++it, end, {Token::TYPE_EQUAL_SIGN, Token::TYPE_COMMA, Token::TYPE_CLOSE_PAREN})) {
                return false;
            }

            switch (it->type) {
                case Token::TYPE_EQUAL_SIGN:
                    if (!::expect(++it, end, Token::TYPE_NUMERIC)) {
                        return false;
                    }

                    li.options[ident] = std::stoi(it->data);

                    if (!::expect(++it, end, {Token::TYPE_COMMA, Token::TYPE_CLOSE_PAREN})) {
                        return false;
                    }

                    if (it->type == Token::TYPE_CLOSE_PAREN) {
                        parsing = false;
                    }

                    break;
                case Token::TYPE_COMMA:
                    li.options[ident] = 0;
                    break;
                case Token::TYPE_CLOSE_PAREN:
                    li.options[ident] = 0;
                    parsing = false;
                    break;
            }
        }

        return true;
    }

	bool consumeLayoutQualifiers(ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
			ShaderInfo::Layout& li) {
		if (!::expect(++it, end, {Token::TYPE_MEMORY_QUALIFIER, Token::TYPE_IN, Token::TYPE_OUT,
				Token::TYPE_UNIFORM, Token::TYPE_BUFFER})) {
			return false;
		}

		while (it->type == Token::TYPE_MEMORY_QUALIFIER) {
			li.memoryQualifiers.push_back(it->data);
			
			if (!::expect(++it, end, {Token::TYPE_MEMORY_QUALIFIER, Token::TYPE_IN, Token::TYPE_OUT,
					Token::TYPE_UNIFORM, Token::TYPE_BUFFER})) {
				return false;
			}
		}

		switch (it->type) {
			case Token::TYPE_IN:
				li.type = ShaderInfo::LayoutType::ATTRIB_IN;
				break;
			case Token::TYPE_OUT:
				li.type = ShaderInfo::LayoutType::ATTRIB_OUT;
				break;
			case Token::TYPE_UNIFORM:
				li.type = li.options.find("std140") != li.options.end()
						? ShaderInfo::LayoutType::UNIFORM_BUFFER : ShaderInfo::LayoutType::UNIFORM;
				break;
			case Token::TYPE_BUFFER:
				li.type = ShaderInfo::LayoutType::SHADER_STORAGE_BUFFER;
				break;
		}

		if (li.type == ShaderInfo::LayoutType::ATTRIB_IN || li.type == ShaderInfo::LayoutType::ATTRIB_OUT
				|| li.type == ShaderInfo::LayoutType::UNIFORM) {
			if (!::expect(++it, end, Token::TYPE_IDENTIFIER)) {
				return false;
			}

			li.typeQualifier = it->data;
		}

		// TODO: I think the buffer name is actually optional for UBOs and SSBOs
		if (!::expect(++it, end, Token::TYPE_IDENTIFIER)) {
			return false;
		}

		li.name = it->data;

		return true;
	}

	bool consumeLayoutVariables(ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
			ShaderInfo::Layout& li) {
		if (!::expect(++it, end, Token::TYPE_OPEN_CURLY)) {
			return false;
		}

		ShaderInfo::Variable var;

		while (it->type != Token::TYPE_CLOSE_CURLY) {
			if (!::expect(++it, end, {Token::TYPE_IDENTIFIER, Token::TYPE_CLOSE_CURLY})) {
				return false;
			}

			if (it->type == Token::TYPE_CLOSE_CURLY) {
				break;
			}

			var.typeName = it->data;

			if (!::expect(++it, end, Token::TYPE_IDENTIFIER)) {
				return false;
			}

			var.name = it->data;

			if (!::expect(++it, end, {Token::TYPE_OPEN_SQUARE, Token::TYPE_SEMI_COLON})) {
				return false;
			}

			if (it->type == Token::TYPE_OPEN_SQUARE) {
				var.isArray = true;

				if (!::expect(++it, end, {Token::TYPE_NUMERIC, Token::TYPE_CLOSE_SQUARE})) {
					return false;
				}

				if (it->type == Token::TYPE_NUMERIC) {
					var.arraySize = std::stoi(it->data);

					if (!::expect(++it, end, Token::TYPE_CLOSE_SQUARE)) {
						return false;
					}
				}
				else {
					var.arraySize = -1;
				}

				if (!::expect(++it, end, Token::TYPE_SEMI_COLON)) {
					return false;
				}
			}
			else {
				var.isArray = false;
			}

			li.body.push_back(var);
		}

		do {
			if (!::expect(++it, end, {Token::TYPE_SEMI_COLON, Token::TYPE_IDENTIFIER})) {
				return false;
			}
		}
		while (it->type != Token::TYPE_SEMI_COLON);

		return true;
	}

    bool expect(const ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            Token::TokenType type) {
        if (it == end) {
            DEBUG_LOG("Shader Parser", LOG_ERROR, "Unexpected token: expected %s got EOF",
                    ::stringifyTokenType(type));
            return false;
        }
        else if (it->type != type) {
            DEBUG_LOG("Shader Parser", LOG_ERROR, "Unexpected token: expected %s got %s (line %d)",
                    ::stringifyTokenType(type), ::stringifyTokenType(it->type), it->line);
            return false;
        }

        return true;
    }

    bool expect(const ArrayList<Token>::iterator& it, const ArrayList<Token>::iterator& end,
            std::initializer_list<Token::TokenType> types) {
        if (it == end) {
            DEBUG_LOG("Shader Parser", LOG_ERROR, "Unexpected token: expected token got EOF");
            return false;
        }

        for (auto& type : types) {
            if (it->type == type) {
                return true;
            }
        }

        DEBUG_LOG("Shader Parser", LOG_ERROR, "Unexpected token: expected %s got %s (line %d)",
                ::stringifyTokenType(*types.begin()), ::stringifyTokenType(it->type), it->line);

        return false;
    }

    const char* stringifyTokenType(enum Token::TokenType type) {
        switch (type) {
            case Token::TYPE_IDENTIFIER:
                return "identifier";
            case Token::TYPE_NUMERIC:
                return "numeric literal";
            case Token::TYPE_OPERATOR:
                return "operator";
            case Token::TYPE_LAYOUT:
                return "layout";
            case Token::TYPE_IN:
                return "in";
            case Token::TYPE_OUT:
                return "out";
            case Token::TYPE_UNIFORM:
                return "uniform";
            case Token::TYPE_BUFFER:
                return "buffer";
            case Token::TYPE_MEMORY_QUALIFIER:
                return "memory qualifier";
            case Token::TYPE_OPEN_PAREN:
                return "(";
            case Token::TYPE_CLOSE_PAREN:
                return ")";
            case Token::TYPE_POUND_SIGN:
                return "#";
            case Token::TYPE_EQUAL_SIGN:
                return "=";
            case Token::TYPE_COMMA:
                return ",";
            case Token::TYPE_SEMI_COLON:
                return ";";
            case Token::TYPE_OPEN_CURLY:
                return "{";
            case Token::TYPE_CLOSE_CURLY:
                return "}";
            case Token::TYPE_OPEN_SQUARE:
                return "[";
            case Token::TYPE_CLOSE_SQUARE:
                return "]";
            default:
                return "invalid token";
        }
    }
};
