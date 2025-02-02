#include "TFormula.h"

void TFormula::make_postfix()
{
	if (current_state != CHECK_DONE) return;
	for (int i = 0; i < arr.size(); ++i){
			TStack<Lexeme_operation*> stack;
		for (int j = 0; j < arr[i].size(); ++j) {
			if (dynamic_cast<Lexeme_real*>(arr[i][j])) post_arr[i].push_back(arr[i][j]);
			else {
				Lexeme_operation* op = dynamic_cast<Lexeme_operation*> (arr[i][j]);
				if (stack.IsEmpty()) stack.push(op);
				else if (op->code == open_bracket) stack.push(op);
				else if (op->code == close_bracket) {
					while (stack.top()->code != open_bracket) {
						post_arr[i].push_back(static_cast<Lexeme*>(stack.top()));
						stack.pop();
					}
					stack.pop();
				}
				else if (op->code==op_set) stack.push(op);
				else if (op->priority >  stack.top()->priority) stack.push(op);
				else {
					post_arr[i].push_back(static_cast<Lexeme*>(stack.top()));
					stack.pop();
					stack.push(op);
				}
			}
		}
		while (!stack.IsEmpty()) {
			post_arr[i].push_back(static_cast<Lexeme*>(stack.top()));
			stack.pop();
		}
	}
	current_state = POSTFIX_DONE;
}

TFormula::TFormula() 
{
	current_state = WAIT_FOR_INPUT;
	make_op_list();

}

TFormula::~TFormula()
{
	clear();
	for (auto it = op_list.begin(); it != op_list.end(); ++it) delete it->second;
}

bool TFormula::check_exp()
{
	arr.resize(orig_exp_arr.size());
	post_arr.resize(orig_exp_arr.size());
	for (int i = 0; i < orig_exp_arr.size(); ++i) {
		TStack<char> brackets;
		std::string& orig_exp = orig_exp_arr[i];
		int k = 0;
		bool wait_for_num = false;
		for (; k < orig_exp.size(); ++k) {
			// ��������� 0
			if (current_state == ORIGIN_STATE) {
				if (orig_exp[k] >= '0' && orig_exp[k] <= '9') {
					real tmp = (orig_exp[k] - '0');
					int j = k + 1; real after_dot = 0.1;
					for (bool dot = false; (j < orig_exp.size()) && ((orig_exp[j] >= '0' && orig_exp[j] <= '9') || (orig_exp[j] == '.' || orig_exp[j] == ',')); ++j) {
						if (orig_exp[j] == '.' || orig_exp[j] == ',') {
							if (!dot) {
								dot = true;
								if (!((j + 1) < orig_exp.size() && orig_exp[j + 1] >= '0' && orig_exp[j + 1] <= '9')) { current_state = ERROR;  return false; }
							}
							else { current_state = ERROR;  return false; }
						}
						else if (!dot) {
							tmp = tmp * 10 + (orig_exp[j] - '0');
						}
						else {
							tmp = tmp + after_dot * (orig_exp[j] - '0');
							after_dot *= 0.1;
						}
					}
					k = j - 1;
					Lexeme* num = new Lexeme_real(tmp);
					arr[i].push_back(num);
					wait_for_num = false;
					current_state = WAIT_FOR_OP;
				}
				else if (orig_exp[k] == '(') {
					brackets.push('(');

					arr[i].push_back(op_list["("]);
				}
				else if (orig_exp[k] == '-' || orig_exp[k] == '+') {
					if (k + 1 >= orig_exp.size()) { current_state = ERROR; k = orig_exp.size(); }
					else {
						if (orig_exp[k] == '-') {
							arr[i].push_back(op_list["u-"]);
						}
						else {
							arr[i].push_back(op_list["u+"]);
						}
					}
				}
				else if ((orig_exp[k] >= 'A' && orig_exp[k] <= 'Z') || (orig_exp[k] >= 'a' && orig_exp[k] <= 'z')) {
					std::string name=make_name(orig_exp,k);
					if (op_list.find(name) != op_list.end()) { arr[i].push_back(op_list[name]); wait_for_num = true; }
					else if (name_list.find(name) != name_list.end()) {
						arr[i].push_back(name_list[name]); 
						if (k + 1 < orig_exp.size() && orig_exp[k + 1] == '='){
							arr[i].push_back(op_list["="]);
							++k;
						}
						else current_state = WAIT_FOR_OP;
					}
					else if (k + 1 < orig_exp.size() && orig_exp[k+1] == '=') {
						Lexeme* num = new Lexeme_var(name);
						name_list.emplace(name, num);
						arr[i].push_back(num);
						arr[i].push_back(op_list["="]);
						++k;
					}
					else { current_state = ERROR; k = orig_exp.size(); }
				}
				else { current_state = ERROR; k = orig_exp.size(); }
			}
			//�������� ���� �����, �������� ��������
			else if (current_state == WAIT_FOR_OP) {
				if (wait_for_num) {
					current_state = ERROR; k = orig_exp.size();
				}
				else if (orig_exp[k] == '+' || orig_exp[k] == '-' || orig_exp[k] == '*' || orig_exp[k] == '/' || orig_exp[k] == '^') {
					std::string op = { orig_exp[k] };
					arr[i].push_back(op_list[op]);
					int j = k + 1;
					if (j >= orig_exp.size()) { current_state = ERROR; k = orig_exp.size(); }
					else if (orig_exp[j] == '+' || orig_exp[j] == '-' || orig_exp[j] == '*' || orig_exp[j] == '/' || orig_exp[j] == '^') { current_state = ERROR; k = orig_exp.size(); }
					else current_state = ORIGIN_STATE;
				}
				else if (orig_exp[k] == ')') {
					arr[i].push_back(op_list[")"]);
					if (brackets.IsEmpty()) { current_state = ERROR; k = orig_exp.size(); }
					else if (brackets.top() == '(') brackets.pop();
					else { current_state = ERROR; k = orig_exp.size(); }
				}
				else { current_state = ERROR; k = orig_exp.size(); }
			}
		}
		if (current_state == ERROR || !(brackets.IsEmpty()) || wait_for_num) {
			current_state = ERROR; return false;
		}
		else current_state = ORIGIN_STATE;
	}
	if (current_state == ORIGIN_STATE) current_state=CHECK_DONE;
	return current_state==CHECK_DONE;
}

real TFormula::calc()
{
	if (current_state != POSTFIX_DONE) return 0.0;
	real res = 0.0;
	if (post_arr.size() != 0) {
		TStack<real> stack;
		TQueue <Lexeme_var*> var_queue;
		for (int k = 0; k < post_arr.size(); ++k) {
			for (int j = 0; j < post_arr[k].size(); ++j) {
				if (dynamic_cast<Lexeme_real*>(post_arr[k][j])) { 
					stack.push(static_cast<Lexeme_real*>(post_arr[k][j])->a);
					if (dynamic_cast<Lexeme_var*>(post_arr[k][j])) var_queue.push(static_cast<Lexeme_var*>(post_arr[k][j]));
				}
				else {
					Lexeme_operation* op = dynamic_cast<Lexeme_operation*> (post_arr[k][j]);
					if (op->code >= op_un_plus) {
						switch (op->code)
						{
						case op_un_min:
							stack.top() = -stack.top();
							break;
						case op_exp:
							stack.top() = exp(stack.top());
							break;
						case op_cos:
							stack.top() = cos(stack.top());
							break;
						case op_sin:
							stack.top() = sin(stack.top());
							break;
						case op_ln:
							if (stack.top() <= 0.0) throw std::exception();
							stack.top() = log(stack.top());
							break;
						default:
							break;
						}
						
					}
					else {
						real tmp = stack.top();
						stack.pop();
						switch (op->code)
						{
						case op_plus:
							stack.top() += tmp;
							break;
						case op_minus:
							stack.top() -= tmp;
							break;
						case op_mult:
							stack.top() *= tmp;
							break;
						case op_div:
							if (tmp == 0.0) throw std::exception();
							stack.top() /= tmp;
							break;
						case op_pow:
							stack.top() = pow(stack.top(), tmp);
							break;
						case op_set:
							stack.top() = tmp;
							var_queue.top()->a = tmp;
							var_queue.pop();
							break;
						default:
							break;
						}
					}
				}
			}
			res = stack.top();
		}
	}
	return res;
}

real& TFormula::add_var(const std::string& name, real num)
{
	if (name_list.find(name) == name_list.end()) {
		Lexeme* pLexeme_var = new Lexeme_var(name, num);
		name_list.emplace(name, pLexeme_var);
		return static_cast<Lexeme_real*>(pLexeme_var)->a;
	}
	else {
		Lexeme_real* pLexeme_var = static_cast<Lexeme_real*> (name_list[name]);
		pLexeme_var->a = num;
		return pLexeme_var->a;
	}
}

void TFormula::clear()
{
	for (int i = 0; i < arr.size(); ++i) {
		for (auto elem : arr[i])
			if (dynamic_cast<Lexeme_real*>(elem) && (!(dynamic_cast<Lexeme_var*>(elem))))
				delete elem;
	}
	for (auto it = name_list.begin(); it != name_list.end(); ++it) delete it->second;
	orig_exp_arr.clear();
	arr.clear();
	post_arr.clear();
}

void TFormula::show_lex()
{
	for (int k = 0; k < arr.size();++k) {
		for(int j=0;j<arr[k].size();++j) std::cout << *arr[k][j] << ' ';
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void TFormula::show_postfix()
{
	for (int k = 0; k < post_arr.size(); ++k) {
		for (int j = 0; j < post_arr[k].size(); ++j) std::cout << *post_arr[k][j] << ' ';
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

std::string TFormula::make_name(std::string& a, int& j)
{
	std::string name;
	for (; j < a.size() && ((a[j] >= 'A' && a[j] <= 'Z') || (a[j] >= 'a' && a[j] <= 'z')); ++j) {
		name.push_back(a[j]);
	}
	--j;
	return name;
}

void TFormula::make_op_list()
{
	std::string op_arr[] = { "(" ,")" ,"=","+" ,"-" ,"*" ,"/" ,"^" ,"u+" ,"u-" ,"exp","cos","sin" ,"ln" };
	for (int i = 0; i < 14; ++i) {
		Lexeme* op = new Lexeme_operation (static_cast<operation_enum>(i));
		op_list.emplace(op_arr[i], op);
	}
	
}

Lexeme_operation::Lexeme_operation(const operation_enum& op)
{
	code = op;
	if (op == open_bracket || op == close_bracket) priority = 0;
	else if(op==op_set) priority = 1;
	else if (op == op_plus || op == op_minus) priority = 2;
	else if (op == op_mult || op == op_div) priority = 3;
	else if (op == op_pow) priority = 4;
	else priority = 5;
}

std::ostream& operator<<(std::ostream& out,const Lexeme& lex)
{
	if (const Lexeme_operation * op = dynamic_cast<const Lexeme_operation*> (&lex))
		out << *op;
	else if (const Lexeme_var * var = dynamic_cast<const Lexeme_var*> (&lex))
		out << *var;
	else if (const Lexeme_real * num = dynamic_cast<const Lexeme_real*> (&lex))
		out << *num;
	else throw std::exception();
	return out;
}

std::istream& operator>>(std::istream& inp, TFormula& form)
{
	form.clear();
	while (!inp.eof()) {
		std::string st;
		inp >> st;
		if(!inp.eof()) form.orig_exp_arr.push_back(st);
	}
	form.current_state = ORIGIN_STATE;
	return inp;
}

std::ostream& operator<<(std::ostream& out, const Lexeme_operation& op)
{
	switch (op.code)
	{
	case open_bracket:
		out << '(';
		break;
	case close_bracket:
		out << ')';
		break;
	case op_plus: op_un_plus:
		out << '+';
		break;
	case op_minus: op_un_min:
		out << '-';
		break;
	case op_mult:
		out << '*';
		break;
	case op_div:
		out << '/';
		break;
	case op_pow:
		out << '^';
		break;
	case op_cos:
		out << "cos";
		break;
	case op_sin:
		out << "sin";
		break;
	case op_exp:
		out << "exp";
		break;
	case op_ln:
		out << "ln";
		break;
	case op_set:
		out << "=";
		break;
	default:
		throw std::exception();
		break;
	}
	return out;
}

