# recursive split of line
# expand xxxx {aaa|bbb} yyyy
# to two lines:
#  xxxx aaa yyyy
#  xxxx bbb yyyy
#
# nesting allowed

BEGIN {
	PROCINFO["re_syntax"] = "RE_SYNTAX_GNU_AWK"
        if (!stderr) stderr="/dev/stderr"
}

{ print ambsplit($0) }

function ambsplit(src, SEP,   nl,na,a1,c,left,right,ambstr,a,out) {
        if (!SEP) SEP="\n"
        #a1=match(src,"{")
        if (0== a1=match(src,"{") ) return src

        left=substr(src,1,a1-1)

        for (c=a1+1;c<=length(src);c++) {
                char=substr(src,c,1)
                if ( char=="{" ) nl++
                if ( char=="|" && !nl) {
                        ambstr[++na]=substr(src,a1+1,c-a1-1)
                        a1=c 
                }
                if ( char=="}" && !(nl--) ) {
                        right=substr(src,c+1)
                        ambstr[++na]=substr(src,a1+1,c-a1-1)
                        for ( a in ambstr ) {
                                out=( out ? out SEP : "") ambsplit(left ambstr[a] right) 
                        } return out
                }
        }
        return ambsplit(src"}")
} 
