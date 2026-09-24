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

// ---------------------------------------------------------------------------
// 标识符缩写。交互库要和选手的翻译单元链接，所以公开接口里出现的名字、关键字、
// 预处理行里的名字、以及任何被 `::` 限定的名字都原样保留，其余内部名字按出现
// 频率从高到低换成最短的可用短名，纯粹为了缩小源文件体积。
// ---------------------------------------------------------------------------
inline const std::set<str>& keywords(){
  static const std::set<str> k={
    "alignas","alignof","and","and_eq","asm","auto","bitand","bitor","bool","break",
    "case","catch","char","char16_t","char32_t","class","compl","const","constexpr",
    "const_cast","continue","decltype","default","delete","do","double","dynamic_cast",
    "else","enum","explicit","export","extern","false","float","for","friend","goto",
    "if","inline","int","long","mutable","namespace","new","noexcept","not","not_eq",
    "nullptr","operator","or","or_eq","private","protected","public","register",
    "reinterpret_cast","return","short","signed","sizeof","static","static_assert",
    "static_cast","struct","switch","template","this","thread_local","throw","true",
    "try","typedef","typeid","typename","union","unsigned","using","virtual","void",
    "volatile","wchar_t","while","xor","xor_eq"};
  return k;
}

// 第 index 个短名（长度为 1 的先排完，再排长度为 2 的）。
str short_name(u4 index){
  static const str alphabet="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const u4 base=52;
  u4 length=1,count=base;
  while(index>=count){index-=count;++length;count*=base;}
  str out(length,'a');
  for(u4 i=0;i<length;++i){out[length-1-i]=alphabet[index%base];index/=base;}
  return out;
}

// 收集 [b,e) 区间里的标识符。
void collect_ids(const str&s,size_t b,size_t e,std::set<str>&out){
  for(size_t i=b;i<e;){
    if(isdigit((unsigned char)s[i])||(s[i]=='.'&&i+1<e&&isdigit((unsigned char)s[i+1]))){
      while(i<e&&(id(s[i])||s[i]=='.'))++i;
      continue;
    }
    if((isalpha((unsigned char)s[i])||s[i]=='_')&&id(s[i])){
      size_t j=i;while(j<e&&id(s[j]))++j;
      out.insert(s.substr(i,j-i));i=j;
    }else ++i;
  }
}

// 数字字面量（0x/0b、小数、指数、u/ll/f 后缀）整体跳过。
size_t number_end(const str&s,size_t i){
  const size_t n=s.size();
  if(s[i]=='.')++i;
  while(i<n&&(isdigit((unsigned char)s[i])||s[i]=='_'))++i;
  if(i<n&&s[i]=='.'){++i;while(i<n&&(isdigit((unsigned char)s[i])||s[i]=='_'))++i;}
  if(i<n&&(s[i]=='e'||s[i]=='E'||s[i]=='p'||s[i]=='P')){
    size_t j=i+1;if(j<n&&(s[j]=='+'||s[j]=='-'))++j;
    if(j<n&&isdigit((unsigned char)s[j])){i=j;while(i<n&&isdigit((unsigned char)s[i]))++i;}
  }
  while(i<n&&id(s[i]))++i;
  return i;
}

str shorten(const str&s,const std::set<str>&extra){
  const size_t n=s.size();
  std::set<str> keep=extra;
  keep.insert(keywords().begin(),keywords().end());
  for(const str&x:{"main","std","size_t","ptrdiff_t","int8_t","uint8_t","int16_t","uint16_t",
    "int32_t","uint32_t","int64_t","uint64_t","intptr_t","uintptr_t","max_align_t","va_list",
    "HUGE_VAL","HUGE_VALF","HUGE_VALL","INFINITY","NAN","NULL","EOF","FILE","errno","assert",
    "isnan","isfinite","isinf","signbit","printf","fprintf","memcpy","memmove","memset",
    "strlen","powl","expl","logl","sqrtl"})keep.insert(x);
  // 预处理行整行原样保留（宏名、include 名都不能动）。
  {
    bool cont=false;
    for(size_t i=0;i<n;){
      size_t e=s.find('\n',i);if(e==str::npos)e=n;
      size_t p=i;while(p<e&&(s[p]==' '||s[p]=='\t'))++p;
      const bool line=(cont||(p<e&&s[p]=='#'));
      if(line){
        collect_ids(s,i,e,keep);
        cont=e>i&&s[e-1]=='\\';
      }else cont=false;
      i=e+1;
    }
  }
  // 词法扫描：字符串/字符字面量原样保留，标识符记录频率与是否被 `::` 限定。
  std::map<str,u4> freq;
  std::set<str> qualified,under;
  for(size_t i=0;i<n;){
    const char c=s[i];
    if(c=='"'||c=='\''){
      const char q=c;++i;
      while(i<n){const char d=s[i++];if(d=='\\'&&i<n)++i;else if(d==q)break;}
      continue;
    }
    if(isdigit((unsigned char)c)||(c=='.'&&i+1<n&&isdigit((unsigned char)s[i+1]))){
      i=number_end(s,i);continue;
    }
    if((isalpha((unsigned char)c)||c=='_')&&id(c)){
      size_t j=i;while(j<n&&id(s[j]))++j;
      const str name=s.substr(i,j-i);
      ++freq[name];
      if(name[0]=='_')under.insert(name);
      bool q=false;
      size_t p=i;while(p>0&&ws(s[p-1]))--p;
      // `::name`、`obj.name`、`ptr->name`、`((attr)`：都可能指向文件外的定义。
      if(p>=2&&s[p-1]==':'&&s[p-2]==':')q=true;
      if(p>=1&&s[p-1]=='.'&&p>=2&&(id(s[p-2])||s[p-2]==')'||s[p-2]==']'))q=true;
      if(p>=2&&s[p-1]=='>'&&s[p-2]=='-')q=true;
      if(p>=2&&s[p-1]=='('&&s[p-2]=='(')q=true;
      size_t r=j;while(r<n&&ws(s[r]))++r;
      if(r+1<n&&s[r]==':'&&s[r+1]==':')q=true;
      if(q)qualified.insert(name);
      i=j;continue;
    }
    ++i;
  }
  keep.insert(qualified.begin(),qualified.end());
  keep.insert(under.begin(),under.end());
  // 候选：内部名字按出现频率从高到低吃最短的短名。
  std::vector<std::pair<u4,str>> order;                 // (频率, 名字)
  for(const auto&kv:freq)if(!keep.count(kv.first))order.push_back({kv.second,kv.first});
  std::sort(order.begin(),order.end(),[](const std::pair<u4,str>&a,const std::pair<u4,str>&b){
    if(a.first!=b.first)return a.first>b.first;
    return a.second<b.second;
  });
  std::map<str,str> re;
  std::set<str> used=keep;
  u4 next=0;
  for(const auto&it:order){
    str cand;
    while(true){
      cand=short_name(next++);
      if(!used.count(cand)&&!keywords().count(cand))break;
    }
    used.insert(cand);
    re[it.second]=cand;
  }
  // 调试用：把“原名→短名”的映射写出来（构建脚本自身排查冲突时很方便）。
  if(const char*dump=getenv("MAL_SHORTEN_MAP")){
    std::ofstream f(dump);
    for(const auto&kv:re)f<<kv.first<<' '<<kv.second<<'\n';
  }
  // 输出。
  str o;o.reserve(n);
  for(size_t i=0;i<n;){
    const char c=s[i];
    if(c=='"'||c=='\''){
      const char q=c;o+=c;++i;
      while(i<n){const char d=s[i++];o+=d;if(d=='\\'&&i<n)o+=s[i++];else if(d==q)break;}
      continue;
    }
    if(isdigit((unsigned char)c)||(c=='.'&&i+1<n&&isdigit((unsigned char)s[i+1]))){
      const size_t j=number_end(s,i);o.append(s,i,j-i);i=j;continue;
    }
    if((isalpha((unsigned char)c)||c=='_')&&id(c)){
      size_t j=i;while(j<n&&id(s[j]))++j;
      const str name=s.substr(i,j-i);
      const auto f=re.find(name);
      o+=(f==re.end()?name:f->second);
      i=j;continue;
    }
    o+=c;++i;
  }
  return o;
}

// 公开接口里出现的标识符：这些名字会出现在选手的翻译单元里，必须保留。
std::set<str> public_ids(const str&header){
  std::set<str> out;
  if(file(header)){const str text=read(header);collect_ids(text,0,text.size(),out);}
  return out;
}

// 文档代码块里出现过的名字（选手会照着抄的那些声明）也一律保留：
// 这样缩过名的交互库和 README 承诺的符号完全对得上。
void doc_ids(const str&root,std::set<str>&out){
  std::vector<str> docs={"/README.md","/README_EN.md","/TUTORIAL.md","/examples/T793310/README.md"};
  for(const str&f:ls(root+"/bundles")){
    if(!isdir(root+"/bundles/"+f))continue;
    docs.push_back("/bundles/"+f+"/README.md");
  }
  for(const str&d:docs){
    const str path=root+d;
    if(!file(path))continue;
    const str text=read(path);
    size_t i=0;
    while((i=text.find("```cpp",i))!=str::npos){
      const size_t b=text.find('\n',i);
      const size_t e=text.find("```",b==str::npos?i:b+1);
      if(b==str::npos||e==str::npos)break;
      collect_ids(text,b,e,out);
      i=e+3;
    }
  }
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
  const str fn=norm(out+"/interactive_lib.cpp");
  write(fn,build(fs));
  fprintf(stderr,"build: %s\n",fn.c_str());
  // 题目用的交互库：只打包远程接口和它的依赖，并把库内部标识符缩成短名。
  // 公开名字（接口声明 + 文档里出现过的）原样保留，保证和选手的翻译单元
  // 以及 README 承诺的符号一致。
  str proj=project();
  if(proj.empty())proj=dir(norm(in));
  const str entry=norm(in+"/remote/impl.hpp");
  const str example=norm(proj+"/examples/T793310/interactive_lib.cpp");
  if(file(entry)&&isdir(dir(example))){
    std::set<str> keep=public_ids(norm(in+"/remote/interface.hpp"));
    doc_ids(proj,keep);
    const str small=norm(example+".tmp");
    write(small,shorten(build({entry}),keep));
    rename(small.c_str(),example.c_str());
    fprintf(stderr,"build: %s (only the modules the interface needs)\n",example.c_str());
  }
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
