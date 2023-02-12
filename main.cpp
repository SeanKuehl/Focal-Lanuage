#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Variable {
public:
	string variableType = "";
	string name = "";
	string value = "";

	Variable(string type, string name, string value) {
		this->variableType = type;
		this->name = name;
		this->value = value;
	}

	string GetValue() {
		return value;
	}

	string GetName() {
		return name;
	}
};

class Token {
public:
	string variableType = "";
	string name = "";
	string kind = "";

	Token(string type, string name, string kind) {
		this->variableType = type;
		this->name = name;
		this->kind = kind;
	}

	string GetKind() {
		return kind;
	}

	string GetName() {
		return name;
	}

	string GetType() {
		return variableType;
	}
};

vector<Variable> userVariables;

bool HasName(string rawToken) {
	int underscorePosition = rawToken.find("_");
	if (isalpha(rawToken.at(underscorePosition + 1))) {
		// it has a name and is a variable
		return true;
	} else {
		// it has a value and is a primitive
		return false;
	}
}

vector<int> ContainsOperation(vector<Token> list) {
	vector<int> operations = {};

	for (int i = 0; i < list.size(); i++) {
		if (list.at(i).GetKind() == "operation") {
			operations.push_back(i);
		}
	}
	if (operations.size() > 0) {
		return operations;
	} else {
		operations = {-1};
		return operations;
	}
}

int ExtractIntValue(Token integer) {
	if (integer.GetKind() == "primitive") {
		// name is value
		return stoi(integer.GetName());
	} else {
		// it's a variable, search variables
		for (int i = 0; i < userVariables.size(); i++) {
			if (userVariables.at(i).GetName() == integer.GetName()) {
				return stoi(userVariables.at(i).GetValue());
			}
		}
	}
}

Token HandleInt32Operation(Token firstInt, Token secondInt, Token operation) {
	int firstIntValue = ExtractIntValue(firstInt);
	int secondIntValue = ExtractIntValue(secondInt);

	if (operation.GetName() == "plus") {
		int temp = firstIntValue + secondIntValue;
		return Token("int32", to_string(temp), "primitive");
	} else if (operation.GetName() == "minus") {
		int temp = firstIntValue - secondIntValue;
		return Token("int32", to_string(temp), "primitive");
	}
}

vector<Token> HandleOperation(int index, vector<Token> line) {
	Token toReplace = Token("none", "none", "none");
	if (line.at(index - 1).GetType() == line.at(index + 1).GetType()) {
		// operations can continue successfully
		if (line.at(index - 1).GetType() == "int32") {
			toReplace = HandleInt32Operation(
				line.at(index - 1), line.at(index + 1), line.at(index));
			line.erase(line.begin() + (index - 1), line.begin() + (index + 2));
			line.insert(line.begin() + (index - 1), toReplace);
		}

		return line;

	} else {
		// type mismatch error
		cout << "Types do not match, operation cannot be performed" << endl;
		exit(1);
	}
}

vector<string> GetTypeAndName(string rawToken) {
	string type = "";
	string name = "";
	int underscorePosition = rawToken.find("_");
	int finalPosition = 0;

	// for parameters, last char is ':'
	if (rawToken.find(':') != string::npos) {
		finalPosition = rawToken.size() - 5; // what it takes to get that colon
	} else {
		finalPosition = rawToken.size() - 1;
	}

	type = rawToken.substr(0, underscorePosition);
	name = rawToken.substr(underscorePosition + 1, finalPosition);

	vector<string> toReturn = {type, name};
	return toReturn;
}

bool Contains(string toFind, vector<string> list) {
	for (int i = 0; i < list.size(); i++) {
		if (list.at(i) == toFind) {
			return true;
		}
	}
	return false;
}

vector<string> Lexer(string rawLine) {
	string token = "";
	bool stringActive = false;
	vector<string> tokens;

	for (int i = 0; i < rawLine.size(); i++) {
		if (rawLine.at(i) == '"' && stringActive == false) {
			stringActive = true;
			token += rawLine.at(i);

		} else if (rawLine.at(i) == '"' && stringActive == true) {
			stringActive = false;
			token += rawLine.at(i);
		} else {
			if (rawLine.at(i) != ' ') {
				token += rawLine.at(i);
			} else if (rawLine.at(i) == ' ' && stringActive == true) {
				token += rawLine.at(i);
			} else if (rawLine.at(i) == ' ' && stringActive == false) {
				tokens.push_back(token);
				token = "";
			}
		}
	}
	// line may not always end with space and may leave a hanging token
	if (token != "") {
		tokens.push_back(token);
	}

	return tokens;
}

vector<Token> Parser(vector<string> rawTokens) {
	vector<string> languageFunctions = {"declare_variable", "call_function"};
	vector<string> operations = {"plus", "minus"};
	vector<string> types = {"int32", "any"};
	vector<Token> tokenList;

	for (int i = 0; i < rawTokens.size(); i++) {
		if (rawTokens.at(i) == languageFunctions.at(0) ||
			rawTokens.at(i) == languageFunctions.at(1)) {
			// it's an language function
			tokenList.push_back(
				Token("none", rawTokens.at(i), "language function"));
		} else if (Contains(rawTokens.at(i), operations)) {
			tokenList.push_back(Token("none", rawTokens.at(i), "operation"));
		} else if (
			rawTokens.at(i).find("_") != string::npos &&
			rawTokens.at(i).find(":") != string::npos) {
			// it's a parameter
			vector<string> info = GetTypeAndName(rawTokens.at(i));
			tokenList.push_back(Token(info.at(0), info.at(1), "parameter"));
		} else if (
			rawTokens.at(i).find("_") != string::npos &&
			HasName(rawTokens.at(i))) {
			// it's a variable
			vector<string> info = GetTypeAndName(rawTokens.at(i));
			tokenList.push_back(Token(info.at(0), info.at(1), "variable"));
		} else if (
			rawTokens.at(i).find("_") != string::npos &&
			HasName(rawTokens.at(i)) == false) {
			// it's a primitive
			vector<string> info = GetTypeAndName(rawTokens.at(i));
			// name holds value
			tokenList.push_back(Token(info.at(0), info.at(1), "primitive"));
		} else {
			// for now assume it's a name
			tokenList.push_back(Token("none", rawTokens.at(i), "name"));
		}
	}

	return tokenList;
}

void ExecuteLine(vector<Token> tokens) {
	vector<Token> line = tokens;
	vector<int> operationsToHandle = ContainsOperation(line);
	while (operationsToHandle.at(0) != -1) {
		line = HandleOperation(operationsToHandle.at(0), line);
		operationsToHandle = ContainsOperation(line);
	}

	if (line.at(0).GetName() == "declare_variable") {
		Variable newVar = Variable(
			line.at(2).GetType(), line.at(1).GetName(), line.at(2).GetName());
		userVariables.push_back(newVar);
	} else if (line.at(0).GetName() == "call_function") {
		if (line.at(1).GetName() == "print") {
			if (line.at(3).GetKind() == "variable") {
				// search variables to print

				for (int i = 0; i < userVariables.size(); i++) {
					if (userVariables.at(i).GetName() == line.at(3).GetName()) {
						cout << userVariables.at(i).GetValue() << endl;
					}
				}
			} else if (line.at(3).GetKind() == "primitive") {
				cout << line.at(3).GetName() << endl; // name is it's val
			}
		}
	}
}

int main() {
	// Create a text string, which is used to output the text file
	string myText;

	// Read from the text file
	ifstream MyReadFile("test.txt");
	vector<string> lines;

	// Use a while loop together with the getline() function to read the file
	// line by line
	while (getline(MyReadFile, myText)) {
		// Output the text from the file

		lines.push_back(myText);
	}

	// Close the file
	MyReadFile.close();

	vector<vector<Token>> toExecute;

	toExecute.push_back(Parser(Lexer(lines.at(0))));
	toExecute.push_back(Parser(Lexer(lines.at(1))));

	for (int i = 0; i < toExecute.size(); i++) {
		ExecuteLine(toExecute.at(i));
	}

	return 0;
}