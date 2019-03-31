#include <iostream> // cout, endl
#include <string> // string, stof
#include <vector> // vector
#include <stack> // stack
#include <queue> // queue
#include <stdexcept> // exception
#include <utility> // move
#include <functional> // function
#include <map> // map
#include <optional> // optional

#include <cmath> // NAN, sin, cos
#include <cctype> // isdigit, isalpha


using std::cout;
using std::endl;

enum class Token_Type
{
    plus, minus, mul, div, pow, mod,
    open_parenthesis, close_parenthesis,
    assignment_op, constant, variable, function, number, comma,
    end_of_tokens,
    unknown
};

std::string token_type_to_str(Token_Type type)
{
    switch (type)
    {
        case Token_Type::plus: return "plus";
        case Token_Type::minus: return "minus";
        case Token_Type::mul: return "mul";
        case Token_Type::div: return "div";
        case Token_Type::pow: return "pow";
        case Token_Type::mod: return "mod";
        case Token_Type::open_parenthesis: return "open_parenthesis";
        case Token_Type::close_parenthesis: return "close_parenthesis";
        case Token_Type::assignment_op: return "=";
        case Token_Type::constant: return "constant";
        case Token_Type::variable: return "variable";
        case Token_Type::function: return "function";
        case Token_Type::number: return "number";
        case Token_Type::comma: return "comma";
        case Token_Type::end_of_tokens: return "end_of_tokens";
        case Token_Type::unknown: return "unknown";
        default: throw std::runtime_error("Name not computed!");
    }
}

enum class Operator_Assoc
{
    L_R, R_L, NONE
};


struct Token
{
    Token_Type type = Token_Type::unknown;

    unsigned col = 1;

    float num = NAN;
    std::string text = "";
    unsigned precedence = 0;
    Operator_Assoc assoc = Operator_Assoc::NONE;

    void *fn;

    bool is_operator()
    {
        bool res = (type == Token_Type::plus  ||
                    type == Token_Type::minus ||
                    type == Token_Type::mul   ||
                    type == Token_Type::div   ||
                    type == Token_Type::pow   ||
                    type == Token_Type::mod);

        return res;
    }

    bool is_operand()
    {
        bool res = (type == Token_Type::number   ||
                    type == Token_Type::constant ||
                    type == Token_Type::variable);

        return res;
    }

    bool is_parenthesis()
    {
        bool res = (type == Token_Type::open_parenthesis ||
                    type == Token_Type::close_parenthesis);

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

    Token* require_next_token(Token_Type requested)
    {
        Token *token = next_token();

        if (token->type != requested)
        {
            std::string error =
                "Expected " + token_type_to_str(requested) + " found " + token_type_to_str(token->type) +
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

std::string read_until_space(const std::string &input)
{
    std::string text;

    for (char c : input)
    {
        if (c == ' ') break;
        text.push_back(c);
    }

    return text;
}

std::map<std::string, void*> functions_map = 
{
    {"sin", &std::sinf},
    {"max", &std::fmaxf}
};

std::map<std::string, float> constants_map = 
{
    {"pi", 3.14},
};

void* is_function(const std::string &text)
{
    void *res = nullptr;

    if (auto it = functions_map.find(text); it != functions_map.end())
    {
        std::pair<std::string, void*> elem = *it;
        res = elem.second;
    }

    return res;
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
            } continue;

            case '^':
            {
                token.type = Token_Type::pow;
                token.text = "^";
                token.assoc = Operator_Assoc::R_L;
                token.precedence = 4;
            } break;

            case '*':
            {
                token.type = Token_Type::mul;
                token.text = "*";
                token.assoc = Operator_Assoc::L_R;
                token.precedence = 3;
            } break;

            case '/':
            {
                token.type = Token_Type::div;
                token.text = "/";
                token.assoc = Operator_Assoc::L_R;
                token.precedence = 3;
            } break;

            case '%':
            {
                token.type = Token_Type::mod;
                token.text = "%";
                token.assoc = Operator_Assoc::L_R;
                token.precedence = 3;
            } break;

            case '+':
            {
                token.type = Token_Type::plus;
                token.text = "+";
                token.assoc = Operator_Assoc::L_R;
                token.precedence = 2;
            } break;

            case '-': // @TODO: handle the unary minus
            {
                token.type = Token_Type::minus;
                token.text = "-";
                token.assoc = Operator_Assoc::L_R;
                token.precedence = 2;
            } break;

            case '=':
            {
                token.type = Token_Type::assignment_op;
                token.text = "=";
            } break;

            case '(':
            {
                token.type = Token_Type::open_parenthesis;
                token.text = "(";
            } break;

            case ')':
            {
                token.type = Token_Type::close_parenthesis;
                token.text = ")";
            } break;

            case ',':
            {
                token.type = Token_Type::comma;
                token.text = ",";
            } break;

            case '!':
            {
                token.type = Token_Type::function;
                token.text = "!";
            } break;

            default:
            {
                if (std::isdigit(c))
                {
                    unsigned len = 0;
                    float num = read_number(input.substr(pos), len);

                    token.type = Token_Type::number;
                    token.num = num;
                    token.text = input.substr(pos, len);
                }
                else if (std::isalpha(c))
                {
                    // function or variable or constant
                    std::string text = read_until_space(input.substr(pos));
                    token.text = text;

                    if (void *fn = is_function(text); fn)
                    {
                        token.type = Token_Type::function;
                        token.fn = fn;
                    }
                    else if (0 /*is_constant(text)*/)
                    {
                        token.type = Token_Type::constant;

                    }
                    else
                    {
                        // it's a variable
                    }
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
    end.type = Token_Type::end_of_tokens;

    res.tokens.push_back(std::move(end));

    return res;
}

void print_input(Tokenizer input)
{
    cout << "input: ";
    while (input.current_token()->type != Token_Type::end_of_tokens)
    {
        Token *t = input.current_token();

        cout << t->text << " ";

        input.next_token();
    }
    cout << endl;
}
void print_operator_stack(std::stack<Token> operator_stack)
{
    cout << "stack: ";
    while (!operator_stack.empty())
    {
        cout << operator_stack.top().text << " ";
        operator_stack.pop();
    }
    cout << endl;
}
void print_output_queue(std::queue<Token> output_queue)
{
    cout << "output: ";
    while (!output_queue.empty())
    {
        cout << output_queue.front().text << " ";
        output_queue.pop();
    }
    cout << endl;
}

// https://en.wikipedia.org/wiki/Shunting-yard_algorithm
std::queue<Token> shunting_yard(Tokenizer &input)
{
    bool debug = true;

    std::queue<Token> output_queue;
    std::stack<Token> operator_stack;
    
    if (debug)
    {
        print_input(input);
        print_operator_stack(operator_stack);
        print_output_queue(output_queue);
        cout << endl;
    }

    Token *current_token = input.current_token();

    while (current_token->type != Token_Type::end_of_tokens)
    {
        if (current_token->type == Token_Type::number)
        {
            output_queue.push(*current_token);
        }
        else if (current_token->type == Token_Type::function ||
                 current_token->type == Token_Type::open_parenthesis)
        {
            operator_stack.push(*current_token);
        }
        else if (current_token->type == Token_Type::comma)
        {
            while (!operator_stack.empty() &&
                   operator_stack.top().type != Token_Type::open_parenthesis)
            {
                output_queue.push(operator_stack.top());
                operator_stack.pop();
            }
            if (operator_stack.empty())
            {
                throw std::runtime_error("misplaced separator or mismatched parentheses");
            }
        }
        else if (current_token->is_operator())
        {
            while (!operator_stack.empty())
            {
                bool cond_1 = operator_stack.top().type == Token_Type::function;

                bool cond_2 = operator_stack.top().is_operator() && 
                              operator_stack.top().precedence > current_token->precedence;

                bool cond_3 = operator_stack.top().is_operator() && 
                              operator_stack.top().precedence == current_token->precedence &&
                              operator_stack.top().assoc == Operator_Assoc::L_R;

                bool cond_4 = operator_stack.top().type != Token_Type::open_parenthesis;
                
                bool cond_1_or_2_or_3 = cond_1 || cond_2 || cond_3;

                if (cond_1_or_2_or_3 && cond_4)
                {
                    output_queue.push(operator_stack.top());
                    operator_stack.pop();
                }
                else
                {
                    break;
                }
            } 

            operator_stack.push(*current_token);
        }
        else if (current_token->type == Token_Type::close_parenthesis)
        {
            while (!operator_stack.empty() &&
                   operator_stack.top().type != Token_Type::open_parenthesis)
            {
                    output_queue.push(operator_stack.top());
                    operator_stack.pop();             
            }

            if (!operator_stack.empty())
            {
                operator_stack.pop(); // pop the '('
            }
            else
            {
                throw std::runtime_error("Mismatched Parentheses!!");
            }
        }
        else
        {
            throw std::runtime_error("Unexpected token: '" + token_type_to_str(current_token->type) + 
                                     "' name: '" + current_token->text + "'");
        }

        current_token = input.next_token();

        if (debug)
        {
            print_input(input);
            print_operator_stack(operator_stack);
            print_output_queue(output_queue);
            cout << endl;
        }
    }

    while (!operator_stack.empty())
    {
        if (operator_stack.top().is_parenthesis())
        {
            throw std::runtime_error("Mismatched Parentheses!!");
        }
        else
        {
            output_queue.push(operator_stack.top());
            operator_stack.pop();
        }

        if (debug)
        {
            print_input(input);
            print_operator_stack(operator_stack);
            print_output_queue(output_queue);
            cout << endl;
        }
    }

    return output_queue;
}

float rpn_evaluaton(std::queue<Token> &queue)
{
    std::stack<Token> stack;

    while (!queue.empty())
    {
        Token &token = queue.front();

        if (token.is_operator() || 
            token.type == Token_Type::function)
        {   
            float tmp = 0;

            if (token.is_operator()) // @NOTE: operator are always binary
            {
                Token b = stack.top();
                stack.pop();

                Token a = stack.top();
                stack.pop();

                switch (token.type)
                {
                    case Token_Type::plus:
                    {
                        tmp = a.num + b.num;
                    } break;

                    case Token_Type::minus:
                    {
                        tmp = a.num - b.num;
                    } break;

                    case Token_Type::mul:
                    {
                        tmp = a.num * b.num;
                    } break;

                    case Token_Type::div:
                    {
                        tmp = a.num / b.num;
                    } break;

                    case Token_Type::mod:
                    {
                        tmp = std::fmodf(a.num, b.num);
                    } break;

                    case Token_Type::pow:
                    {
                        tmp = std::powf(a.num, b.num);
                    } break;

                    default:
                        throw std::runtime_error("Found unexpected token during calculation: '" + token.text + "'");
                }
            }
            else
            {
                if (token.text == "sin")
                {
                    Token b = stack.top();
                    stack.pop();

                    tmp = std::sinf(b.num);
                }
                else if (token.text == "max")
                {
                    Token b = stack.top();
                    stack.pop();

                    Token a = stack.top();
                    stack.pop();

                    tmp = fmaxf(a.num, b.num);
                }
                else if (token.text == "!")
                {
                    Token b = stack.top();
                    stack.pop();

                    tmp = 120.0f; // @FIX: to fix!!
                }

            }
            
            Token new_token;
            new_token.type = Token_Type::number;
            new_token.num = tmp;
            new_token.text = std::to_string(tmp); // @NOTE: forse non serve...

            stack.push(std::move(new_token));
        }
        else if (token.is_operand())
        {
            stack.push(token);
        }
        else
        {
            throw std::runtime_error("Error in the queue, found: '" + token.text + "'");
        }

        queue.pop();
    }

    if (stack.size() != 1)
    {
        throw std::runtime_error("Stack has more than 1 element");
    }

    Token &last = stack.top();
    
    return last.num;
}

float parse(const std::string &input)
{
    Tokenizer infix_notation = tokenize(input); // 1) convert input into tokens

    std::queue<Token> rpn = shunting_yard(infix_notation); // 2) convert tokens to reverse polish notation

    float res = rpn_evaluaton(rpn); // 3) solve the reverse polish notation

    return res;
}

int main()
{

    try
    {
        std::string input;
        input = "2+(3*(8-4))";
        input = "((15 / (7 - (1 + 1))) * 3) - (2 + (1 + 1))";
        input = "sin ( max ( 2, 3 ) / 3 * 3.141592 )";
        input = "3 + 4 * 2 / ( 1 - 5 ) ^ 2 ^ 3";
        input = "3 + ((4 * 2) / (( 1 - 5 ) ^ (2 ^ 3)))";
        input = "5!";
        
        float res = parse(input);
        
        cout << "> " << input << endl;
        cout << ": " << std::fixed << res << endl;

    } catch (std::runtime_error &e)
    {
        cout << "[EXCEPTION] " << e.what() << endl;
    }

    return 0;
}