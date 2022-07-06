//
//      String
//
// More convenient than using character arrays (char*). 
// Its your typical String class - nuff said.
//
//
//

#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include "array.h"
#include "stringclass.h"

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

String::String(){
	buf=new char[1];
	buf[0]='\0';
}

String::String(const String &t){
	buf=NULL;
	if(t.buf) {
		buf=new char[strlen(t.buf)+1];strcpy(buf,t.buf);
	}
}

String::String(const char *s){
	if(!s) s = "";
	buf=new char[strlen(s)+1];
	strcpy(buf,s);
}
String::String(char *s,int len){
	assert(s);
	assert(len>=0);
	buf=new char[len+1];
	strncpy(buf,s,len);
	buf[len] = '\0';
}

String::String(const int a){
	buf=NULL;
	sprintf("%d",a);
}

String::String(const float a){
	buf=NULL;
	sprintf("%f",a);
}


String::~String() {
	delete buf;
	buf=NULL;
}


String &String::operator=(const String& s)
{
    if (buf) {
        delete[] buf;
    }
	buf=NULL;
	if(s.buf) {
		buf = new char[strlen(s.buf)+1];
		strcpy(buf,s.buf);
	}
	return *this;
}

String &String::operator=(const char *s)
{
	char *tmp=buf;
	buf=NULL;
	if(s) { 
		buf = new char[strlen(s)+1];
		strcpy(buf,s);
	}
    if(tmp) {
		// note: we delete tmp last since s may point to part of it
		delete[] tmp;
	}
	return *this;
}

//----------------------------------------------
// note that we return 'char&' not just 'char'
// that means that the caller can modify the
// string by using the return value on the lhs.
char &String::operator[](int i) {
	assert(buf);
	assert(i>=0 && i<= (int)strlen(buf));
	return buf[i];
}

//----------------------------------------------
// Concatenating operators.  Note '+=' is a member.
//
String &String::operator+=(const String &b){
	if(!b.buf){
		return *this;
	}
	if(!buf) {
		(*this)=b;
		return *this;
	}
	char *tmp=buf;
	buf = new char[strlen(tmp) + strlen(b.buf) +1];
	strcpy(buf,tmp);
	strcat(buf,b.buf);
	delete[] tmp;
	return *this;
}
String operator+(const String &a,const String &b){
	String s(a);
	s += b;
	return s; 
}


void String::writeto(int &a) const {
	if(!buf){return;}
	sscanf(buf,"%d",&a);
}

void String::writeto(float &a)  const{
	if(!buf){return;}
	sscanf(buf,"%f",&a);
}

void String::writeto(String &a)  const{
	a=*this;
}


int String::Asint() const{
	int a=0;
	writeto(a);
	return a;
}
float String::Asfloat() const{
	float f=0.0f;
	writeto(f);
	return f;
}

//----------------------------------------------
// The obvious operators for comparing strings
// Dont you wish strcmp() was called strdiff().
//
int operator==(const String &a,const String &b){
	return (!strcmp(a.buf,b.buf));
}
int operator!=(const String &a,const String &b){
	return (strcmp(a.buf,b.buf)); 
}
int operator>(const String &a,const String &b){
	return (strcmp(a.buf,b.buf)>0);  
}
int operator<(const String &a,const String &b){
	return (strcmp(a.buf,b.buf)<0);  
}
int operator>=(const String &a,const String &b){
	return (strcmp(a.buf,b.buf)>=0); 
}
int operator<=(const String &a,const String &b){
	return (strcmp(a.buf,b.buf)<=0); 
}
int operator==(const String &a,const char *b){
	return (!strcmp(a.buf,b));
}
int operator!=(const String &a,const char *b){
	return (strcmp(a.buf,b));
}
int operator> (const String &a,const char *b){
	return (strcmp(a.buf,b)>0);  
}
int operator< (const String &a,const char *b){
	return (strcmp(a.buf,b)<0);  
}
int operator>=(const String &a,const char *b){
	return (strcmp(a.buf,b)>=0); 
}
int operator<=(const String &a,const char *b){
	return (strcmp(a.buf,b)<=0); 
}
int operator==(const char *a,const String &b){
	return (!strcmp(a,b.buf));
}
int operator!=(const char *a,const String &b){
	return (strcmp(a,b.buf));
}
int operator> (const char *a,const String &b){
	return (strcmp(a,b.buf)>0);  
}
int operator< (const char *a,const String &b){
	return (strcmp(a,b.buf)<0);  
}
int operator>=(const char *a,const String &b){
	return (strcmp(a,b.buf)>=0); 
}
int operator<=(const char *a,const String &b){
	return (strcmp(a,b.buf)<=0); 
}

String &String::sprintf(char *format,...) {
	va_list vl;
	va_start(vl, format);
	if (buf) {	
		delete[] buf;
		buf=NULL;
	}
	int  n;
	char *tmp = NULL;
    int tmpsize = 128;
	while( (n = vsnprintf((tmp=new char[tmpsize]), tmpsize-1, format, vl))<0) {
		delete []tmp;
		tmpsize *=2;
	}
    buf = new char[n+1];
	assert(buf);
	strcpy(buf,tmp);
	delete []tmp;
	va_end(vl);
	return *this;
}
	

//------------------------------------------------
// Returns true if c is one of the characters in s
int IsOneOf(const char e, const char *p) {
	while(*p) if(*p++==e) return 1;
	return 0;
}

//------------------------------------------------
// Finds the position of the first character in s
// that isn't in the list of characters in 'delimeter'
const char *SkipChars(const char *s,const char *delimeter) {
	while(*s&& IsOneOf(*s,delimeter)){ s++;}
	return s;
}

const char *SkipToChars(const char *s,const char *tokens) {
	while(*s && !IsOneOf(*s,tokens)) {s++;}
	return s;
}

char *SkipChars(char *s,const char *delimeter) {
	while(*s&& IsOneOf(*s,delimeter)){ s++;}
	return s;
}

char *SkipToChars(char *s,const char *tokens) {
	while(*s && !IsOneOf(*s,tokens)) {s++;}
	return s;
}

String &String::DropFirstChars(char *delimeter){
	if(!buf) {
		return *this;
	}
	(*this) = SkipChars(buf,delimeter);
	return *this;
}
//---------------------------------------------------
// PopFirstWord - removes and returns first word in
// the supplied string.  
// Note that input parameter 'line' will be modified.
String PopFirstWord(String &line,char *delimeter) {
	Array<char> f;
	char *s = line;
	s = SkipChars(s,delimeter);
	while((*s) && !IsOneOf(*s,delimeter)) {
		f.Add(*s);
		s++;
	}
	f.Add('\0');
	s = SkipChars(s,delimeter);
	line = s;
	return String(f.element);		
}

String PopLastWord(String &line) {
	Array<char> f;
	char *s = ((char*)line) + strlen(line);
	assert(! *s);
	while(s>(char*)line && IsOneOf(*(s-1)," \t")) {
		s--;
	}
	f.Add('\0');
	while((s>(char*)line) && !IsOneOf(*(s-1)," \t\n\r")){
		s--;
		f.Insert((*s),0);
	}
	while(s>(char*)line && IsOneOf(*(s-1)," \t")){
		s--;
	}
	*s='\0';
	return String(f.element);		
}

String ToLower(const char *b)
{
	String s(b);
	char *c=(char*)s;
	assert(c);
	while(*c){ *c = tolower(*c); c++;}
	return s;
}

int hashpos(String &s,int number_of_slots)
{
	int sum=0;
	unsigned char *p = (unsigned char*) s.buf;
	while(*p){sum+=*p++;}
	return sum%number_of_slots;  
}


int fileexists(char *filename)
{
	if(!filename || !*filename) return 0;
	FILE *fp;
	fp=fopen(filename,"r");
	if(fp)fclose(fp);
	return (fp!=NULL);
}

String filefind(String filename,String pathlist,String extentions)
{
	if(filename=="") return String("");
	if(fileexists(filename)) return filename;
	String fn;
	String ex=extentions;
	while(ex != "")
	{
		String e = PopFirstWord(ex," .;");
		fn = filename + "." + e;
		if(fileexists(fn)) return fn;
	}
	while(pathlist[0])
	{
		String p = PopFirstWord(pathlist," ;/");
		p += "/";
		fn = p+filename;
		if(fileexists(fn)) return fn;
		String ex=extentions;
		while(ex != "")
		{
			String e = PopFirstWord(ex," .;");
			fn = p + filename + "." + e;
			if(fileexists(fn)) return fn;
		}
	}
	return String("");
}




