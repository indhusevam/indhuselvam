// Copyright 2012 The Go Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "a.h"

/*
 * Helpers for building cmd/gc.
 */

// gcopnames creates opnames.h from go.h.
// It finds the OXXX enum, pulls out all the constants
// from OXXX to OEND, and writes a table mapping
// op to string.
void
gcopnames(char *dir, char *file)
{
	char *p, *q;
	int i, j, end;
	Buf in, b, out;
	Vec lines, fields;
	
	binit(&in);
	binit(&b);
	binit(&out);
	vinit(&lines);
	vinit(&fields);
	
	bwritestr(&out, bprintf(&b, "// auto generated by go tool dist\n"));
	bwritestr(&out, bprintf(&b, "static char *opnames[] = {\n"));

	readfile(&in, bprintf(&b, "%s/go.h", dir));
	splitlines(&lines, bstr(&in));
	i = 0;
	while(i<lines.len && !contains(lines.p[i], "OXXX"))
		i++;
	end = 0;
	for(; i<lines.len && !end; i++) {
		p = xstrstr(lines.p[i], "//");
		if(p != nil)
			*p = '\0';
		end = contains(lines.p[i], "OEND");
		splitfields(&fields, lines.p[i]);
		for(j=0; j<fields.len; j++) {
			q = fields.p[j];
			if(*q == 'O')
				q++;
			p = q+xstrlen(q)-1;
			if(*p == ',')
				*p = '\0';
			bwritestr(&out, bprintf(&b, "	[O%s] = \"%s\",\n", q, q));
		}
	}
	
	bwritestr(&out, bprintf(&b, "};\n"));

	writefile(&out, file, 0);

	bfree(&in);
	bfree(&b);
	bfree(&out);
	vfree(&lines);
	vfree(&fields);
}

// mkanames reads [568].out.h and writes anames[568].c
// The format is much the same as the Go opcodes above.
void
mkanames(char *dir, char *file)
{
	int i, ch;
	Buf in, b, out;
	Vec lines;
	char *p;

	binit(&b);
	binit(&in);
	binit(&out);
	vinit(&lines);

	ch = file[xstrlen(file)-3];
	bprintf(&b, "%s/../cmd/%cl/%c.out.h", dir, ch, ch);
	readfile(&in, bstr(&b));
	splitlines(&lines, bstr(&in));
	
	// Include link.h so that the extern declaration there is
	// checked against the non-extern declaration we are generating.
	bwritestr(&out, bprintf(&b, "#include <u.h>\n"));
	bwritestr(&out, bprintf(&b, "#include <libc.h>\n"));
	bwritestr(&out, bprintf(&b, "#include <bio.h>\n"));
	bwritestr(&out, bprintf(&b, "#include <link.h>\n"));
	bwritestr(&out, bprintf(&b, "\n"));

	bwritestr(&out, bprintf(&b, "char*	anames%c[] = {\n", ch));
	for(i=0; i<lines.len; i++) {
		if(hasprefix(lines.p[i], "\tA")) {
			p = xstrstr(lines.p[i], ",");
			if(p)
				*p = '\0';
			p = xstrstr(lines.p[i], "\n");
			if(p)
				*p = '\0';
			p = lines.p[i] + 2;
			bwritestr(&out, bprintf(&b, "\t\"%s\",\n", p));
		}
	}
	bwritestr(&out, "};\n");
	writefile(&out, file, 0);

	bfree(&b);
	bfree(&in);
	bfree(&out);
	vfree(&lines);
}