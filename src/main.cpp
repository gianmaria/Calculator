#include <iostream> // cout, endl
#include <string> // string, stof
#include <vector> // vector
#include <stdexcept> // exception
#include <utility> // move
#include <functional> // function

#include <cmath> // NAN
#include <cctype> // isdigit, isalpha

using std::cout;
using std::endl;

enum class TokenType
{
    plus, minus, mul, div, pow, mod, fact,
    open_parenthesis, close_parenthesis,
    assignment_op, variable, function, number,
    end_of_tokens,
    unknown
};
enum class operator_associativity
{
    L_R, R_L
};

std::string token_type_to_str(TokenType type)
{
    switch (type)
    {
        case TokenType::plus: return "plus";
        case TokenType::minus: return "minus";
        case TokenType::mul: return "mul";
        case TokenType::div: return "div";
        case TokenType::pow: return "pow";
        case TokenType::mod: return "mod";
        case TokenType::fact: return "fact";
        case TokenType::open_parenthesis: return "open_parenthesis";
        case TokenType::close_parenthesis: return "close_parenthesis";
        case TokenType::assignment_op: return "=";
        case TokenType::variable: return "variable";
        case TokenType::function: return "function";
        case TokenType::number: return "number";
        case TokenType::end_of_tokens: return "end_of_tokens";
        case TokenType::unknown: return "unknown";
        default: throw std::runtime_error("Name not computed!");
    }
}

struct Token
{
    TokenType type = TokenType::unknown;

    unsigned col = 1;

    float num = NAN;
    std::string text = "";
    unsigned precedence = 0;
    operator_associativity associativity;

    std::function<float(float)> fn;

    bool operator==(TokenType expected) const
    {
        bool res = (expected == type);
        return res;
    }

    bool operator!=(TokenType expected) const
    {
        bool res = (expected != type);
        return res;
    }
};

struct Tokenizer
{
    std::string input;
    std::vector<Token> tokens;

    unsigned current_token_idx = 0;
    unsigned old_token_idx = 0;

    void save_state()
    {
        if (old_token_idx != 0)
        {
            throw std::runtime_error("Only one save at a time!");
        }

        old_token_idx = current_token_idx;
    }

    void restore_state()
    {
        current_token_idx = old_token_idx;
        old_token_idx = 0;
    }

    Token* current_token()
    {
        Token *token = nullptr;

        if (current_token_idx < tokens.size())
        {
            token = &tokens[current_token_idx];
        }
        else
        {
            token = &tokens.back(); // last token is always end_of_tokens
        }
        return token;
    }

    Token* next_token()
    {
        ++current_token_idx;

        Token *token = current_token();

        return token;
    }

    Token* peek_token()
    {
        ++current_token_idx;

        Token *token = current_token();

        --current_token_idx;

        return token;
    }

    Token* prev_token(unsigned back = 1)
    {
        Token *token = nullptr;

        unsigned desired_token_idx = current_token_idx - back;

        if (desired_token_idx < current_token_idx) // check for underflow
        {
            token = &tokens[desired_token_idx];
        }
        else
        {
            token = &tokens.front();
        }
        return token;
    }

    Token* require_next_token(const TokenType &type)
    {
        Token *token = next_token();

        if (*token != type)
        {
            std::string error = 
                "Expected " + token_type_to_str(type) + " found " + token_type_to_str(token->type) + 
                " Col:" + std::to_string(token->col);
            throw std::runtime_error(error);
        }

        return token;
    }
};

float read_number(const std::string &input, unsigned &len)
{
    len = 0;

    while (std::isdigit(input[len]) || 
           input[len] == '.')
    {
        ++len;
    }
    
    std::string num_to_parse = input.substr(0, len);

    float num = std::stof(num_to_parse);

    return num;
}

Tokenizer tokenize(const std::string &input)
{
    Tokenizer res;
    res.input = input;

    unsigned pos = 0;
    while (pos < input.length())
    {
        Token token;
        token.col = pos + 1;

        char c = input[pos];

        switch (c)
        {
            case ' ':
            {
                while (pos < input.length() &&
                       input[pos] == ' ')
                {
                    ++pos;
                }
            } continue; // @NOTE: is this necessary?

            case '^':
            {
                token.type = TokenType::pow;
                token.text = "^";
                token.associativity = operator_associativity::R_L;
                token.precedence = 1;
            } break;


            case '!':
            {
                token.type = TokenType::fact;
                token.text = "!";
                token.associativity = operator_associativity::L_R; // @NOTE: non molto convinto
                token.precedence = 1;
            } break;

            case '*':
            {
                token.type = TokenType::mul;
                token.text = "*";
                token.associativity = operator_associativity::L_R;
                token.precedence = 2;
            } break;

            case '/':
            {
                token.type = TokenType::div;
                token.text = "/";
                token.associativity = operator_associativity::L_R;
                token.precedence = 2;
            } break;

            case '%':
            {
                token.type = TokenType::mod;
                token.text = "%";
                token.associativity = operator_associativity::L_R;
                token.precedence = 2;
            } break;

            case '+':
            {
                token.type = TokenType::plus;
                token.text = "+";
                token.associativity = operator_associativity::L_R;
                token.precedence = 3;
            } break;

            case '-':
            {
                token.type = TokenType::minus;
                token.text = "-";
                token.associativity = operator_associativity::L_R;
                token.precedence = 3;
            } break;
        
            case '=':
            {
                token.type = TokenType::assignment_op;
                token.text = "=";
            } break;

            default:
            {
                if (std::isdigit(c))
                {
                    unsigned len = 0;
                    float num = read_number(input.substr(pos), len);

                    token.type = TokenType::number;
                    token.num = num;
                    token.text = input.substr(pos, len);
                }
                else if (std::isalpha(c))
                {
                    // function or variable
                    throw std::runtime_error("Not yet implemented!");
                }
                else
                {
                    throw std::runtime_error("Unrecognized token: '" + std::string(1, c) + "'");
                }
            }
        }

        pos += (unsigned)token.text.length();

        res.tokens.push_back(std::move(token));
    }

    Token end;
    end.type = TokenType::end_of_tokens;

    res.tokens.push_back(std::move(end));

    return res;
}

Tokenizer shunting_yard(Tokenizer &infix_notation)
{
    Tokenizer res;
    res.input = infix_notation.input;

    const Token *token = infix_notation.current_token();

    while (*token != TokenType::end_of_tokens)
    {

        token = infix_notation.next_token();
    }

    return res;
}

float parse(const std::string &input)
{
    Tokenizer infix_notation = tokenize(input); // 1) convert input into tokens

    Tokenizer rpn = shunting_yard(infix_notation); // 2) convert tokens to reverse polish notation

#if 0
    float res = rpn_evaluaton(rpn); // 3) solve the reverse polish notation
#endif

    return 7;
}

int main()
{
    std::string input = "123 124 * 5";

    float res = parse(input);

    cout << "> " << input << endl;
    cout << res << endl;

    return 0;
}