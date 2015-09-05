grammar S3;

options {
	language=Cpp;
	//backtrack=true;
	//memoize=true;
	output=AST;
}

tokens {
    BLOCK = 'block';
}

@lexer::includes 
{
#include "UserMiniTraits.hpp"
}
@lexer::namespace{ Antlr3Mini }

@parser::includes {
#include "UserMiniTraits.hpp"
#include "S3Lexer.hpp"
}
@parser::namespace{ Antlr3Mini }
@parser::members {
	class Evaluator {
        public:
        	Evaluator(const char*text) : m_text(text) {};
        	Evaluator(std::string text) : m_text(text) {};
        	std::string const& toString() { return m_text; };
        private:
            std::string m_text;
    };
}

start_rule returns[S3Parser::Evaluator *evaluator, std::string message]
	: r=parse
        {
            $evaluator = $r.evaluator;
            $message = $r.message;
        } 
	;

parse returns[S3Parser::Evaluator *evaluator, std::string message]
  :  r=receive
        {
            S3Parser::Evaluator *e = $r.evaluator;
            std::string m = $r.message;
            $evaluator   = $r.evaluator;
            $message = $r.message;
        }
  ;

receive returns[S3Parser::Evaluator *evaluator, std::string message]
  :  RECEIVE f=FILENAME
        {
            $evaluator = new S3Parser::Evaluator($f.text);
            $message = "Some message here...";
        }
  ;

RECEIVE: 'RECEIVE';
FILENAME: 'FILENAME';
		
