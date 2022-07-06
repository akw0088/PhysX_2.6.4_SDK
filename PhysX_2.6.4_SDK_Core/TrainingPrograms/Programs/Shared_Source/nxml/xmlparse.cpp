//
//               xmlparse
//
// A very simple parser for loading
// valid xml into a useful tree data structure.
// This code does not do xml validation and therefore
// isn't that forgiving with broken input.
//
// There is also some extra facilities here to
// auto create binary structures from ascii data and vice versa.
// 
// If the xml node has an attribute called "ctype"
// then this code uses that attribute's value to automatically convert 
// ascii to/from binary on input/output.  This is very handy
// for large chunks of organized data such as 3D model mesh data.
// 

#include "xmlparse.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#define strcasecmp stricmp
#endif

static void indent(FILE *fp,int d) { for(int i=0;i<d;i++) fprintf(fp," ");}


XMLElement::~XMLElement() {
	int i;
	for(i=0;i<children.count;i++) {
		delete children[i];
	}
	children.count=0;
	free(data);
}

static int Size(char c)
{
	if(c=='f') return sizeof(float);
	if(c=='d') return sizeof(int);
	if(c=='c') return sizeof(char);
	if(c=='b') return sizeof(char);
	if(c=='h') return sizeof(short);
	if(c=='p') return sizeof(char*);
	if(c=='z') return sizeof(int);
	if(c==' ') return 0;
	assert(0);
	return 0;
}

int CalcSize(const char *ctype)
{
	int i,size =0;
	for(i=0;i<(int)strlen(ctype);i++)
	{
		size += Size(ctype[i]);
	}
	return size;
}

unsigned char *AscToBin(const char *p,const int count,const char *ctype,void *_data)
{
	int i,j;
	if(count==0) {return NULL;	}
	int size=CalcSize(ctype);
	unsigned char *data = (unsigned char*)_data;
	if(data==NULL) data = (unsigned char*)malloc(size*count);
	assert(data);
	int w = strlen(ctype);
	unsigned char *d = data;
	static Array<char *> entry_format;
	static Array<int>    entry_size;
	entry_format.count=0;
	entry_size.count=0;
	for(j=0;j<w;j++)
	{
		switch(ctype[j])
		{
			case 'f': entry_format.Add("%f%n")  ;break;
			case 'd': entry_format.Add("%d%n")  ;break;
			case 'p': entry_format.Add("%d%n")  ;break;
			case 'c': entry_format.Add("%c%n")  ;break;
			case 'b': entry_format.Add("%d%n")  ;break;
			case 'h': entry_format.Add("%hd%n") ;break;
			case 'z': entry_format.Add(NULL)    ;break;
			case ' ': entry_format.Add(NULL)    ;break;
			default: assert(0);
		}
		entry_size.Add(Size(ctype[j]));
	}
	static Array<char> tmp;
	for(i=0;i<count;i++)
	{
		for(j=0;j<w;j++)
		{
			if(ctype[j]==' ') continue;
			tmp.count=0;
			p=SkipChars(p," ;,\t\n\r");
			assert(*p); // there should be more to input
			while(*p && !IsOneOf(*p," ,;\n\r\t"))
			{
				tmp.Add(*p++);
			}
			tmp.Add('\0');
			if(!entry_format[j]) 
			{
				for(int k=0;k<entry_size[j];k++){*d++=0;}
				continue;
			}
			if(ctype[j]=='b')
			{
				int td,offset;
				sscanf(tmp.element,entry_format[j],&td,&offset) ==1||(assert(0),0);
				*d = (unsigned char) td;
			}
			else
			{
                int offset;
				int rc = sscanf(tmp.element,entry_format[j],d,&offset);
				if(rc!=1)
				{
					if(ctype[j]=='d' && (tmp.element[0]=='f' || tmp.element[0]=='F'))
					{
						(*(int*)d) = 0;
					}
					else if(ctype[j]=='d' && (tmp.element[0]=='t' || tmp.element[0]=='T'))
					{
						(*(int*)d) = 1;
					}
					else
					{	
						assert(0);
					}
				}
				
			}
			if(ctype[j]=='p') 
			{
				int offset = (*((int*)d) * size);
				*((unsigned char**) d) = (offset==-size || offset==0)?NULL:(data + offset);   // todo: remove offset==0 when all my data files are updated.
			}
			d+= entry_size[j]; // Size(ctype[j]);
		}
	}
	p=SkipChars(p," ;,\t\n\r");
	assert(*p=='\0');  // nothing should be left
	assert(d == data+size*count);
	return data;
}

void *XMLExtractData(XMLElement *e)
{
	if(e->data) return e->data;
	if(!e->attributes.Exists("ctype") || !e->attributes.Exists("count")) return NULL;
	e->data = AscToBin(e->body,e->attributes["count"].Asint(),e->attributes["ctype"],NULL);
	return e->data;
}

void OutputData(XMLElement *elem,FILE *fp,int depth)
{
	int i,j;
	if(!elem->data) return;
	String &ctype = elem->ctype;
	int size=CalcSize(ctype);
	int w = strlen(ctype);
	int n = elem->count;
	unsigned char *data = (unsigned char *) elem->data;
	unsigned char *d = data;
	if(n>1) {indent(fp,depth);}
	for(i=0;i<n;i++)
	{
		if(i && w>1) indent(fp,depth);
		for(j=0;j<w;j++)
		{
			if(ctype[j]==' ') continue;
			if(j) fprintf(fp," ");
			switch(ctype[j])
			{
				case 'f': fprintf(fp,"%g" ,*((float*)d));break;
				case 'd': fprintf(fp,"%d" ,*((int *) d));break;
				case 'c': fprintf(fp,"%c" ,*((float*)d));break;
				case 'h': fprintf(fp,"%hd",*((short*)d));break;
				case 'z': break;
				case 'p': 
					{
						unsigned char *dp = *((unsigned char **) d);
						fprintf(fp,"%d" ,dp?(dp-data)/size:-1);
						break;
					}
				default: assert(0);
			}
			d+= Size(ctype[j]);
		}
		if(n>1) fprintf(fp,",%s",(i==n-1 || w>1)?"\n":"  ");
	}
	assert(d == data+CalcSize(ctype)*n);
}

static int countnodes(char *n,int l,int r)
{
	if(!n) return 0;
	return 1+ countnodes(*(char**)(n+l),l,r)+countnodes(*(char**)(n+r),l,r);
}
static unsigned char *startdata;
static unsigned char *curdata;
static void fillit(char *n,int l,int r,int size)
{
	if(!n) return;
	memcpy(curdata,n,size);
	void **lp = (void**)(curdata+l);
	void **rp = (void**)(curdata+r);
	curdata+=size;
	*lp = (*lp)?curdata:NULL;
	fillit(*(char**)(n+l),l,r,size);
	*rp = (*rp)?curdata:NULL;
	fillit(*(char**)(n+r),l,r,size);
}
unsigned char *Linearize(void *root,int l,int r,int size,char *ctype,int *pcount)
{
	if(ctype)
	{
		assert(CalcSize(String(ctype))==size);
	}
	int count = countnodes((char*)root,l,r);
	*pcount=count;
	unsigned char* data = (unsigned char*)malloc(size*count);
	curdata   = data;
	startdata = data;
	fillit((char*)root,l,r,size);
	assert(curdata-data==size*count);
	return data;
}

static char *s,*t;
static String token("");
String &NextToken()
{
	s = SkipChars(s," \t\n\r");
	t = s;
	if(!*s) {
		token = String(""); 
		return token;
	}
	if(*t=='\"') {
		s=t=t+1;
		s = SkipToChars(s,"\"");
		token = String(t,s-t);
		if(*s) s++;
		return token;
	}
	if(IsOneOf(*t,"<>!?=/")) {
		s++;
		token = String(t,1);
		return token;
	}
	s = SkipToChars(s,"<>!?=/ \r\t\n");
	token = String(t,s-t);
	s = SkipChars(s," \t\n\r");
	return token;
}

static XMLElement *XMLParse() {
	while(token != "<" || IsOneOf(*s,"!?")) {
		NextToken();
		assert(*s);
	}
	XMLElement *elem = new XMLElement();
	elem->tag = NextToken();
	while(*s && NextToken() != ">" && token != "/") {
		String &newval = elem->attributes[token];
		NextToken();
		assert(token == "=");
		newval = NextToken();
	}
	if(token == "/") {
		NextToken();
		assert(token == ">");
		return elem; // no children
	}
	assert(token == ">");
	NextToken();
	while( token !="<" || *s!='/') {
		if(token == "<") {
			elem->children.Add(XMLParse());
		}
		else {
			s = SkipToChars(s,"<");
			elem->body += String(t,s-t);
			NextToken();
		}
	}
	assert(*t=='<');assert(*s=='/');
	NextToken();
	NextToken();
	assert(token == elem->tag);
	NextToken();
	assert(token ==">");
	NextToken();
	return elem;
}

XMLElement *XMLParseData(char *buf, int len) {
	char *end = buf+len;
	s = t = buf;
	return XMLParse();
}

XMLElement *XMLParseFile(const char *filename) {
	char *buf;
	if(!filename || !*filename) return NULL;
	FILE *fp = fopen(filename,"r");
	if(!fp) fp = fopen(String(filename)+".xml","r");
	if(!fp) return NULL;
	assert(fp);
	fseek(fp,0,SEEK_END);
	long int len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	buf = new char[len+1];
	assert(buf);
	int rc = fread(buf,1,len,fp);
	//assert(rc==len);
	buf[rc]='\0';
	char crap[16];
	rc=fread(crap,1,16,fp);
	assert(rc<=0);
	XMLElement *e = XMLParseData(buf,len);
	delete buf;
	return e;
}


void XMLSaveFile(XMLElement *elem,FILE *fp)
{
	static int depth=0;
	if(elem->attributes.Exists("ctype") ) elem->ctype = elem->attributes["ctype"];
	if(elem->attributes.Exists("count") ) elem->count = elem->attributes["count"].Asint();
	int singleline = (elem->children.count==0 && strlen(elem->body)<60 && elem->count<2); 
	int i;
	indent(fp,depth);
	fprintf(fp,"<%s",elem->tag);
	for(i=0;i<elem->attributes.slots_count;i++)if(elem->attributes.slots[i].used) {
		fprintf(fp," %s=\"%s\"",elem->attributes.slots[i].key,elem->attributes.slots[i].value);
	}
	fprintf(fp,(singleline)?">":">\n");
	depth+=2;
	for(i=0;i<elem->children.count;i++) {
		XMLSaveFile(elem->children[i],fp);
	}
	if(strlen((char*)elem->body)) {
		if(!singleline) indent(fp,depth);
		fprintf(fp,"%s%s",(char*)elem->body,(singleline)?"":"\n");
	}
	if(elem->data)
	{
		OutputData(elem,fp,depth);
	}
	depth-=2;
	if(!singleline) indent(fp,depth);
	fprintf(fp,"</%s>\n",elem->tag);
}
void XMLSaveFile(XMLElement *elem,const char *filename)
{
	FILE *fp = fopen(filename,"w");
	assert(fp);
	XMLSaveFile(elem,fp);
	fclose(fp);
}


//------------- reflection --------------

ClassDesc::ClassDesc(const String &n):name(n){classdesc[ToLower(n)] = this;}
ClassDesc::~ClassDesc(){classdesc.Delete(ToLower(name));}
Hash<String,ClassDesc*> classdesc(-1);
void *LoadMember(XMLElement *c,void *p,ClassDesc *d)
{
	int i;
	for(i=0;i<d->members.count;i++)
	{
		ClassDesc::memberdef &m= d->members[i];
		if(!strcasecmp(c->tag,m.name)) 
		{
			p = ((unsigned char*)p)+m.offset;
			if(m.isarray)
			{
				int count = c->attributes["count"].Asint();
				*(void **)p = AscToBin(c->body,count,m.ctype); 
				((int *)p)[1]=((int *)p)[2]=count;
			}
			else 
			{	
				AscToBin(c->body,1,m.ctype,p); 
			}
			return p;
		}
	} 
	return NULL;
}
 
void *LoadClass(XMLElement *e,void *p,ClassDesc *d)
{
	int i;
	for(i=0;i<e->children.count;i++)
	{
		if(!LoadMember(e->children[i],p,d))
		{
#ifdef DEBUG
			//fprintf(stderr,"WARNING: Unexpected child node <%s> found in <%s>\n",e->children[i]->tag,e->tag);
#endif
		}
	}
	return p;
}

XMLElement *SaveClass(void *base,ClassDesc *d)
{
	int i;
	XMLElement *e = new XMLElement(ToLower(d->name));
	for(i=0;i<d->members.count;i++)
	{
		ClassDesc::memberdef &m= d->members[i];
		XMLElement *c = new XMLElement(ToLower(m.name));
		e->children.Add(c);
		c->ctype = m.ctype;
		void* p = ((unsigned char*)base)+m.offset;
		if(m.isarray)
		{
			int count = ((int *)p)[1];
			c->count = 1;
			c->attributes["count"]=String(count);
			c->data = *((void**)p);
		}
		else 
		{
			c->count=1;
			c->data = p;
		}
	}
	return e;
}

