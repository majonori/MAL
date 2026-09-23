#include<bits/stdc++.h>
#ifdef _WIN32
#include<windows.h>
#else
#include<dirent.h>
#include<sys/stat.h>
#endif

namespace {

typedef unsigned int u4;
typedef unsigned long long u8;
using str=std::string;
std::set<str>vis,sys;
str root="include";
inline bool ws(char c){return c==' '||c=='\t'||c=='\r'||c=='\n'||c=='\f'||c=='\v';}
inline bool id(char c){return isalnum((unsigned char)c)||c=='_'||c=='$';}
str norm(str s){
  for(char&c:s)if(c=='\\')c='/';
  str hd;u4 st=0;if(!s.empty()&&s[0]=='/')hd="/",st=1;else if(s.size()>1&&s[1]==':')hd=s.substr(0,2)+"/",st=s.size()>2&&s[2]=='/'?3:2;
  std::vector<str>a;str t;
  for(size_t i=st,j;i<=s.size();i=j+1){
    j=s.find('/',i);if(j==str::npos)j=s.size();t=s.substr(i,j-i);
    if(t.empty()||t==".")continue;
    if(t==".."&&!a.empty()&&a.back()!="..")a.pop_back();else if(t!=".."||hd.empty())a.push_back(t);
  }s=hd;for(str x:a)s+=(s.empty()||s.back()=='/'?"":"/")+x;return s.empty()?".":s;
}
inline str dir(str s){size_t p=s.find_last_of('/');return p==str::npos?"":s.substr(0,p);}
inline str base(str s){size_t p=s.find_last_of('/');return p==str::npos?s:s.substr(p+1);}
inline bool file(const str&s){std::ifstream f(s.c_str(),std::ios::binary);return f.good();}
#ifdef _WIN32
inline bool isdir(const str&s){DWORD x=GetFileAttributesA(s.c_str());return x!=INVALID_FILE_ATTRIBUTES&&(x&FILE_ATTRIBUTE_DIRECTORY);}
inline void md(const str&s){if(!s.empty()&&s!="."&&!isdir(s)&&!CreateDirectoryA(s.c_str(),0)&&GetLastError()!=ERROR_ALREADY_EXISTS)throw std::runtime_error("cannot mkdir "+s);}
std::vector<str>ls(const str&s){
  std::vector<str>a;WIN32_FIND_DATAA d;HANDLE h=FindFirstFileA((s+"/*").c_str(),&d);if(h==INVALID_HANDLE_VALUE)return a;
  do{str x=d.cFileName;if(x!="."&&x!="..")a.push_back(x);}while(FindNextFileA(h,&d));FindClose(h);std::sort(a.begin(),a.end());return a;
}
#else
inline bool isdir(const str&s){struct stat x;return !stat(s.c_str(),&x)&&S_ISDIR(x.st_mode);}
inline void md(const str&s){if(!s.empty()&&s!="."&&!isdir(s)&&mkdir(s.c_str(),0755)&&errno!=EEXIST)throw std::runtime_error("cannot mkdir "+s);}
std::vector<str>ls(const str&s){
  std::vector<str>a;DIR*d=opendir(s.c_str());if(!d)return a;for(dirent*e;(e=readdir(d));){str x=e->d_name;if(x!="."&&x!="..")a.push_back(x);}closedir(d);std::sort(a.begin(),a.end());return a;
}
#endif
void mkdirs(str s){s=norm(s);if(s=="."||s.empty())return;str h;if(s[0]=='/')h="/";else if(s.size()>2&&s[1]==':')h=s.substr(0,3);for(size_t i=h.size(),j;i<=s.size();i=j+1){j=s.find('/',i);if(j==str::npos)j=s.size();str x=s.substr(i,j-i);if(x.empty())continue;h+=(h.empty()||h.back()=='/'?"":"/")+x;md(h);}}
str read(const str&s){
  std::ifstream f(s.c_str(),std::ios::binary);if(!f)throw std::runtime_error("cannot open "+s);
  std::ostringstream o;o<<f.rdbuf();return o.str();
}
inline bool pre(const str&s,u4 p,const char*t){
  while(p<s.size()&&(s[p]==' '||s[p]=='\t'))++p;
  if(p>=s.size()||s[p++]!='#')return 0;
  while(p<s.size()&&(s[p]==' '||s[p]=='\t'))++p;
  u4 n=strlen(t);return s.compare(p,n,t)==0&&(p+n==s.size()||!id(s[p+n]));
}
str dep(str cur,str x){
  str a=norm((dir(cur).empty()?"":dir(cur)+"/")+x);if(file(a))return a;
  str b=norm(root+"/"+x);if(file(b))return b;
  throw std::runtime_error("include not found: "+x+" in "+cur);
}
str merge(const str&fn){
  str f=norm(fn);if(vis.count(f))return "";vis.insert(f);
  str s=read(f),o,line;std::stringstream ss(s);
  while(std::getline(ss,line)){
    u4 p=0;while(p<line.size()&&(line[p]==' '||line[p]=='\t'))++p;
    if(pre(line,0,"pragma")&&line.find("once")!=str::npos)continue;
    if(pre(line,0,"include")){
      u4 q=line.find("include",p+1);q=q==str::npos?p:q+7;while(q<line.size()&&ws(line[q]))++q;
      if(q<line.size()&&line[q]=='\"'){
        u4 r=line.find('"',q+1);if(r==str::npos)throw std::runtime_error("bad include in "+f);
        o+=merge(dep(f,line.substr(q+1,r-q-1)));continue;
      }
      if(q<line.size()&&line[q]=='<'){
        u4 r=line.find('>',q+1);if(r==str::npos)throw std::runtime_error("bad include in "+f);
        str h=line.substr(q,r-q+1);if(sys.insert(h).second)o+="#include"+h+"\n";continue;
      }
    }o+=line+'\n';
  }return o;
}
inline bool join(char a,char b){
  if(id(a)&&id(b))return 1;
  if((a=='.'&&isdigit((unsigned char)b))||(isdigit((unsigned char)a)&&b=='.'))return 1;
  str x;x+=a;x+=b;
  static const std::set<str>z={"++","--","->","<<",">>","<=",">=","==","!=","&&","||","+=","-=","*=","/=","%=","&=","|=","^=","##","::",".*","//","/*"};
  if(z.count(x))return 1;
  // 有向图（digraph）：%: %:%: %> <: <% :>，少一个空格就会被当成 # [ ] { } 等符号
  static const std::set<str>dg={"%:","%>","<:","<%",":>"};
  return dg.count(x);
}
str pack(const str&s){
  str o;u4 n=s.size(),i=0;bool gap=0;
  auto put=[&](char c){if(gap&&!o.empty()&&join(o.back(),c))o+=' ';gap=0;o+=c;};
  while(i<n){
    if(ws(s[i])){gap=1;++i;continue;}
    if(s[i]=='/'&&i+1<n&&s[i+1]=='/'){while(i<n&&s[i]!='\n')++i;gap=1;continue;}
    if(s[i]=='/'&&i+1<n&&s[i+1]=='*'){i+=2;while(i+1<n&&(s[i]!='*'||s[i+1]!='/'))++i;i=std::min<u4>(n,i+2);gap=1;continue;}
    if(s[i]=='#'){u4 k=i;while(k&&s[k-1]!='\n'&&(s[k-1]==' '||s[k-1]=='\t'||s[k-1]=='\r'))--k;if(k&&s[k-1]!='\n'){put(s[i++]);continue;}
      if(!o.empty()&&o.back()!='\n')o+='\n';
      bool c=1;while(c&&i<n){u4 b=i;while(i<n&&s[i]!='\n')++i;u4 e=i;while(e>b&&s[e-1]=='\r')--e;c=e>b&&s[e-1]=='\\';o.append(s,b,i-b);o+='\n';if(i<n)++i;}gap=0;continue;
    }
    if((s[i]=='R'&&i+1<n&&s[i+1]=='\"')||s[i]=='\"'||s[i]=='\''){
      if(s[i]=='R'){
        if(gap&&!o.empty()&&join(o.back(),'R'))o+=' ';
        gap=0;o+=s[i++];o+=s[i++];
        str d;while(i<n&&s[i]!='('){d+=s[i];o+=s[i++];}if(i<n)o+=s[i++];str e=")"+d+"\"";
        while(i<n){if(s.compare(i,e.size(),e)==0){o+=e;i+=e.size();break;}o+=s[i++];}continue;
      }
      char q=s[i];put(s[i++]);while(i<n){char c=s[i];o+=c;++i;if(c=='\\'&&i<n)o+=s[i++];else if(c==q)break;}continue;
    }
    put(s[i++]);
  }
  while(!o.empty()&&ws(o.back()))o.pop_back();
  o+='\n';return o;
}
inline bool src(const str&s){
  size_t p=s.find_last_of('.');if(p==str::npos)return 0;str x=s.substr(p);return x==".h"||x==".hh"||x==".hpp"||x==".hxx"||x==".c"||x==".cc"||x==".cpp"||x==".cxx";
}
void scan(const str&d,std::vector<str>&ds,std::vector<str>&all){
  std::vector<str>v=ls(d),f;for(str x:v){str p=norm(d+"/"+x);if(isdir(p))scan(p,ds,all);else if(src(x))f.push_back(p),all.push_back(p);}if(!f.empty())ds.push_back(d);
}
std::vector<str>direct(const str&d){std::vector<str>a;for(str x:ls(d)){str p=norm(d+"/"+x);if(!isdir(p)&&src(x))a.push_back(p);}return a;}
str build(const std::vector<str>&a){vis.clear();sys.clear();str s;for(str f:a)s+=merge(f);return pack(s);}
void write(const str&fn,const str&s){mkdirs(dir(fn));std::ofstream f(fn.c_str(),std::ios::binary);if(!f)throw std::runtime_error("cannot write "+fn);f<<s;}
str rel(str p,str r){p=norm(p),r=norm(r);if(p==r)return "";if(p.size()>r.size()&&p.compare(0,r.size(),r)==0&&p[r.size()]=='/')return p.substr(r.size()+1);throw std::runtime_error(p+" is not under "+r);}
void all(str in,str out);
str project(){
  str p=".";
  for(u4 i=0;i<64;i++){
    str q=norm(p+"/include");if(isdir(q))return norm(p);
    p=norm(p+"/..");
  }return "";
}
void autoall(){
  str p=project();if(p.empty())throw std::runtime_error("cannot locate project root (missing include/ in current or parent directories)");
  all(norm(p+"/include"),norm(p+"/bundles"));
}
void all(str in,str out){
  root=norm(in);in=root;out=norm(out);if(!isdir(in))throw std::runtime_error("include directory not found: "+in);
  std::vector<str>ds,fs;scan(in,ds,fs);std::sort(ds.begin(),ds.end());std::sort(fs.begin(),fs.end());if(fs.empty())throw std::runtime_error("no source files under "+in);
  for(str d:ds){str r=rel(d,in),fn=norm(out+(r.empty()?"":"/"+r)+"/main.cpp");write(fn,build(direct(d)));fprintf(stderr,"build: %s\n",fn.c_str());}
  str fn=norm(out+"/interactive_lib.cpp");write(fn,build(fs));fprintf(stderr,"build: %s\n",fn.c_str());
}

} // namespace

int main(int argc,char**argv){
  try{
    if(argc==1){autoall();return 0;}
    if(str(argv[1])=="--all"){
      if(argc>4){fprintf(stderr,"usage: build [--all [include_dir [bundle_dir]]] | <entry> [output]\n");return 1;}
      all(argc>2?argv[2]:"include",argc>3?argv[3]:"bundles");return 0;
    }
    if(argc>3){fprintf(stderr,"usage: build [--all [include_dir [bundle_dir]]] | <entry> [output]\n");return 1;}
    str in=argv[1],out=argc==3?argv[2]:"";if(!file(in)&&file(root+"/"+in))in=root+"/"+in;str s=build({in});
    if(out.empty())fwrite(s.data(),1,s.size(),stdout);else write(out,s);
  }catch(const std::exception&e){fprintf(stderr,"build: %s\n",e.what());return 1;}return 0;
}
