#include <iostream> // cout, endl
#include <string> // string, stof, getline
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

#define _USE_MATH_DEFINES
#include <math.h>

using std::cout;
using std::endl;


#define calc_exception(msg) ( \
    std::runtime_error(std::string("[EXCEPTION]") + std::string("\n") + \
        std::string("File: ") + std::string(__FILE__) + std::string("\n") + \
        std::string("Function: ") + std::string(__func__) + std::string("():") + std::to_string(__LINE__) + std::string("\n") + \
        std::string("Msg: ") + std::string(msg)) \
)

enum class Token_Type
{
    plus, minus, mul, div, pow, mod,
    open_parenthesis, close_parenthesis,
    assignment_op, constant, variable, function, 
	number, comma,
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

        default: throw calc_exception("Name not computed!");
    }
}

enum class Operator_Assoc
{
    L_R, R_L, NONE
};

using token_fn = std::function<float(std::stack<float>&)>;

struct Token
{
    Token_Type type = Token_Type::unknown;
    std::string text = "";

    unsigned col = 1;

    float num = NAN;
    unsigned precedence = 0;
    Operator_Assoc assoc = Operator_Assoc::NONE;

    token_fn fn;

    Token()
    {}
    
    Token(Token_Type type, float num) : type(type), num(num) 
    {}

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
            throw calc_exception("Only one save at a time!");
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
            throw calc_exception(error);
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

std::string read_string(const std::string &input)
{
    std::string text;

    for (char c : input)
    {
        if (std::isalpha(c) ||
            std::isdigit(c) ||
            c == '_')
        {
            text.push_back(c);
        }
        else
        {
            break;
        }
    }

    return text;
}


float do_sin(std::stack<float>& operands)
{
    float op_1 = operands.top();
    operands.pop();

    float rad = (op_1 * (float)M_PI) / 180.0f;
    float res = std::sinf(rad);
    
    return res;
}

float do_cos(std::stack<float>& operands)
{
    float op_1 = operands.top();
    operands.pop();

    float rad = (op_1 * (float)M_PI) / 180.0f;
    float res = std::cosf(rad);

    return res;
}

float do_max(std::stack<float>& operands)
{
    float op_2 = operands.top();
    operands.pop();

    float op_1 = operands.top();
    operands.pop();

    float res = std::fmaxf(op_1, op_2);

    return res;
}

float do_max3(std::stack<float>& operands)
{
    float op_3 = operands.top();
    operands.pop();
    
    float op_2 = operands.top();
    operands.pop();

    float op_1 = operands.top();
    operands.pop();

    float res = std::fmaxf(std::fmaxf(op_1, op_2), op_3);

    return res;
}


float do_fact(std::stack<float>& operands)
{
    float res = 1;

    float op_1 = operands.top();
    operands.pop();

    for (unsigned n = (unsigned)op_1;
         n > 0;
         --n)
    {
        res *= (float)n;
    }

    return res;
}

std::map<std::string, token_fn> functions_map =
{
    {"fact", do_fact},
    {"sin", do_sin},
    {"cos", do_cos},
    {"max3", do_max3},
    {"max", do_max}
};

std::map<std::string, float> constants_map = 
{
    {"PI",  (float)M_PI},
    {"TAU", (float)M_PI*2.0f}
};

// add on operator map?

std::optional<token_fn> is_function(const std::string &text)
{
    std::optional<token_fn> res;

    if (auto it = functions_map.find(text); it != functions_map.end())
    {
        std::pair<std::string, token_fn> pair = *it;
        res = pair.second;
    }

    return res;
}

std::optional<float> is_constant(const std::string &text)
{
    std::optional<float> res;

    if (auto it = constants_map.find(text); it != constants_map.end())
    {
        std::pair<std::string, float> pair = *it;
        res = pair.second;
    }

    return res;
}


Tokenizer tokenize_and_lex(const std::string &input)
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
                    // function, variable or constant
                    std::string text = read_string(input.substr(pos));
                    token.text = text;

                    if (auto fn = is_function(text); fn.has_value())
                    {
                        token.type = Token_Type::function;
                        token.fn = fn.value();
                    }
                    else if (auto cons = is_constant(text); cons.has_value())
                    {
                        token.type = Token_Type::constant;
                        token.num = cons.value();
                    }
                    else
                    {
                        // it's a variable // @TODO: handle variables later, maybe?
						throw calc_exception("Unrecognized string: '" + text + "'");
					}
                }
                else
                {
                    throw calc_exception("Unrecognized token: '" + std::string(1, c) + "'");
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
    /*
    TODOs:
    we can merge:
        else if (current_token->type == Token_Type::comma) and
        else if (current_token->type == Token_Type::close_parenthesis)

    */
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
        if (current_token->type == Token_Type::number || 
            current_token->type == Token_Type::constant)
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
                throw calc_exception("misplaced separator or mismatched parentheses");
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

                // basically we pop from the operator_stack 
                // if, on top of the stack, there are operations with higher priority,
                // compared to the current_token, or we are not inside a parenthesis
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
                operator_stack.pop(); // pop the remaning '('
            }
            else
            {
                throw calc_exception("Mismatched Parentheses!!");
            }
        }
        else
        {
            throw calc_exception("Unexpected token: '" + token_type_to_str(current_token->type) + 
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
            throw calc_exception("Mismatched Parentheses!!");
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
    std::stack<float> operands;

    while (!queue.empty())
    {
        Token &token = queue.front();

        float tmp = 0;

        if (token.is_operator()) // @NOTE: operator are always binary
        {
            float op_2 = operands.top();
            operands.pop();

            float op_1 = operands.top();
            operands.pop();

            switch (token.type)
            {
                case Token_Type::plus:
                {
                    tmp = op_1 + op_2;
                } break;

                case Token_Type::minus:
                {
                    tmp = op_1 - op_2;
                } break;

                case Token_Type::mul:
                {
                    tmp = op_1 * op_2;
                } break;

                case Token_Type::div:
                {
                    tmp = op_1 / op_2;
                } break;

                case Token_Type::mod:
                {
                    tmp = std::fmodf(op_1, op_2);
                } break;

                case Token_Type::pow:
                {
                    tmp = std::powf(op_1, op_2);
                } break;

                default:
                    throw calc_exception("Found unexpected token during calculation: '" + token.text + "'");
            }

            operands.push(tmp);
        }
        else if (token.type == Token_Type::function)
        {
            tmp = token.fn(operands);
            operands.push(tmp);
        }
        else if (token.is_operand())
        {
            tmp = token.num;
            operands.push(tmp);
        }
        else
        {
            throw calc_exception("Error in the queue, found: '" + token.text + "'");
        }

        queue.pop();
    }

    if (operands.size() != 1)
    {
        throw calc_exception("Operand's stack has more than 1 element, something went wrong during the rpn evaluation");
    }

    float res = operands.top();
    
    return res;
}

float calc(const std::string &input)
{
    Tokenizer infix_notation = tokenize_and_lex(input); // 1) convert input into tokens

    std::queue<Token> rpn = shunting_yard(infix_notation); // 2) convert tokens to reverse polish notation

    float res = rpn_evaluaton(rpn); // 3) solve the reverse polish notation

    return res;
}

int main()
{

	while (1)
	{
		try
		{
            cout << "> ";

            std::string expression;
            std::getline(std::cin, expression);

            if (expression == "exit")
            {
                break;
            }
            else if (expression != "")
            {
                float res = calc(expression);
                cout << ": " << std::fixed << res << endl;
            }
        
        } catch (std::runtime_error &e)
		{
			cout << e.what() << endl;
		}
	}

    return 0;
}
