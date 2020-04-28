
; Macroinstructions for defining data structures

macro struct@overridedefs [datadef] {
	struc datadef [val:?] \{ \common
		match any:no,force@alignment:@rstruct \\{ define field@struct -,align,datadef#.align \\}
		define field@struct .,datadef,<val> \}
	macro datadef [val:?] \{ \common
		match any:no,force@alignment:@rstruct \\{ define field@struct -,align,datadef#.align \\}
		define field@struct -,datadef,<val> \} }
;==============================================================
macro struct@overridersvs [datadef] {
	match ddd,def.#datadef \{
	struc datadef count \\{ \\common
		match any:no,force@alignment:@rstruct \\\{ define field@struct -,align,ddd\#.align \\\}
		define field@struct .,ddd,count dup (?) \\}
	macro datadef count \\{ \\common
		match any:no,force@alignment:@rstruct \\\{ define field@struct -,align,ddd\#.align \\\}
		define field@struct -,ddd,count dup (?) \\} \} }
;==============================================================
macro struct@overridealgn [datadef] {
	struc datadef amount:default.#datadef \{ define field@struct -,datadef,amount \}
	macro datadef amount:default.#datadef \{ define field@struct -,datadef,amount \} }
;==============================================================
macro struct@restoredefs [datadef] {
	restruc datadef
	purge datadef }

;==============================================================
; GLOBAL ASSUMPTIONS FOR THIS MACRO SET
; THERE IS NO PRACTICAL SENSE TO ENCLOSE IN UNION OR STRUCT NO ONE FIELD
; THERE IS NO PRACTICAL SENSE TO ENCLOSE IN SUBUNION OR SUBSTRUCT ONLY 1 FIELD OR EVEN NO ONE
; THERE IS NO PRACTICAL SENSE TO ENCLOSE IN SUBUNION ONLY UNNAMED FIELDS
;==============================================================

macro struct definition {
	match name tail, definition: \{
		virtual
			db \`name
			load initial@struct byte from $$
			if initial@struct = '.'
				display 'Error: name of structure should not begin with a dot.',0Dh,0Ah
				err
			end if
		end virtual

		macro ends \\{
			match , sub@struct \\\{
				if $
					display 'Error: definition of ',\`name,' contains illegal instructions.',0Dh,0Ah
					err
				end if

				match any,datadef@directives \\\\{ struct@restoredefs any \\\\}
				match any,datarsv@directives \\\\{ struct@restoredefs any \\\\}
				match any,algnmnt@directives \\\\{ struct@restorealgn any \\\\}

				purge union,struct,rstruct,ends

				irpv fields,field@struct \\\\{
					restore field@struct
				 \\\\common
					restore @struct
					make@struct name,fields
					define fields@\#name fields \\\\}
				end virtual \\\} ;end virtual must be after make@struct
			match any, sub@struct \\\{
				tmp@struct equ field@struct
				restore field@struct
				field@struct equ tmp@struct> \\\}
			restore sub@struct \\}

		;match :,tail \\{ \\}
		match parent:,tail \\{ field@struct equ fields@\\#parent \\} \}

	match any,datadef@directives \{ struct@overridedefs any \}
	match any,datarsv@directives \{ struct@overridersvs any \}
	match any,algnmnt@directives \{ struct@overridealgn any \}
	irp datadef,union,struct,rstruct \{
		macro datadef \\{
			field@struct equ ,sub\#datadef,<
			sub@struct equ sub\#datadef \\} \}

	define @struct
	sub@struct equ
	virtual at 0 }
;==============================================================
macro union definition {
	struct definition
	define @union }
;==============================================================
macro rstruct definition {
	struct definition
	define @rstruct }
;==============================================================
macro make@struct name,[field,type,def] {
    common
	local CONVERTED,PREVIOUS,CURRENT
	define CONVERTED name
	define CURRENT 1
    forward
	PREVIOUS equ CURRENT
	define CURRENT 1
	local sub
	match , field \{ define CURRENT 2
			 make@substruct type,name,sub def \}
	match -, field \{ define CURRENT 0 \}
	match =1:=0, PREVIOUS:CURRENT \{ CONVERTED equ CONVERTED, < <type def>						 \}
	match =0:=0, PREVIOUS:CURRENT \{ CONVERTED equ CONVERTED	      , <type def>			      \}
	match =1:=1, PREVIOUS:CURRENT \{ CONVERTED equ CONVERTED,			    , .#field,	type, <def> \}
	match =0:=1, PREVIOUS:CURRENT \{ CONVERTED equ CONVERTED			   >, .#field,	type, <def> \}
	match =1:=2, PREVIOUS:CURRENT \{ CONVERTED equ CONVERTED,			    , .,	sub,	    \}
	match =0:=2, PREVIOUS:CURRENT \{ CONVERTED equ CONVERTED			   >, .,	sub,	    \}
	match =2, CURRENT	      \{ define CURRENT 1 \}
    common
	local anonymous
	match =0, CURRENT	      \{ CONVERTED equ CONVERTED			   >, anonymous,:,	    \}
	match converted, CONVERTED \{
		match no:no, @union:@rstruct \\{ define@struct converted \\}
		match no:, @union:@rstruct \\{
			restore @rstruct
			define@rstruct converted \\}
		match :no, @union:@rstruct \\{
			restore @union
			define@union converted \\} \} }
;==============================================================
macro define@union name,[anon,field,type,def] {
 forward
	match any, anon \{
		irp anondef,any \\{
			\\local anonsize
			virtual at 0
				anondef
				anonsize = $
			end virtual
			if anonsize > $
				rb anonsize - $
			end if
			\\}  \}
	if ~ field eq .
		virtual at 0
			name#field type def
			sizeof.#name#field = $ - name#field
		end virtual
		if sizeof.#name#field > $
			rb sizeof.#name#field - $
		end if
	else
		virtual at 0
			label name#.#type
			rb sizeof.#type
		end virtual
		if sizeof.#type > $
			rb sizeof.#type - $
		end if
	end if
 common
	sizeof.#name = $
	restruc name
	struc name value \{
		\local \..base
		match , @struct \\{ define field@struct .,name,value \\}
		match no, @struct \\{
			label \..base
			last@union equ
 forward
			match any, last@union \\\{
			virtual at \..base
				field type def
			end virtual \\\}
			match , last@union \\\{
				match , value \\\\{ field type def \\\\}
				match any, value \\\\{ field type value \\\\} \\\}
			last@union equ field
 common
			if sizeof.#name > $ - \..base
				rb sizeof.#name - ($ - \..base)
			end if
			label . at \..base \\}
	\}
	macro name value \{
		\local \..base
		match , @struct \\{ define field@struct -,name,value \\}
		match no, @struct \\{
			label \..base
			last@union equ
 forward
			match , last@union \\\{ match any, value \\\\{ field type value \\\\} \\\}
			last@union equ field
 common
			if sizeof.#name > $ - \..base
				rb sizeof.#name - ($ - \..base)
			end if \\} \} }
;==============================================================
macro define@rstruct name,[anon,field,type,def] {
 common
	local list,..start,..size
	list equ
	virtual at -..size
		..start:
 forward
		local anonsize
;!!! WE MAKING ASSUMPTION THAT UNNAMED FIELDS ARE JUST RESERVE SPACE
		anonsize = $
		match any, anon \{ irp anondef,any \\{ anondef \\} \}
		anonsize = $-anonsize
;    END OF ASSUMPTION
		if ~ field eq .
			name#field type def
			sizeof.#name#field = $ - name#field
		else
			label name#.#type
			rb sizeof.#type
		end if
		local value
		match any, list \{ list equ list, \}
		list equ list <value>
 common
		..size = $ - ..start
	end virtual
	sizeof.#name = ..size
	restruc name
	match values, list \{
		struc name value \\{
			match , @struct \\\{ define field@struct .,name,<values> \\\}
			match no, @struct \\\{
 forward
;!!! WE MAKING ASSUMPTION THAT UNNAMED FIELDS ARE JUST RESERVE SPACE
				rb anonsize
;    END of ASSUMPTION
				match , value \\\\{ field type def \\\\}
				match any, value \\\\{
					field type value
					if ~ field eq . 			  ; MAIN PURPOSE OF THEESE 3 LINES - REALISATION OF LEGACY ALIGNMENT STYLE IN STRUCTURES
						rb sizeof.#name#field - ($-field) ; BECAUSE ALIGNED FIELD PRECIDED WITH ANOTHER SHORTER SIZE FIELD - WE CAN EMULATE ALIGNMENT BY ADDING SEQUENCE OF ",?" TO END OF THAT SHORTER SIZE FIELD
					end if					  ; 2ND PURPOSE - COMPARISON THAT DATA PASSED TO INITIALIZE STRUCTURE WILL FIT IN ITS MEMBERS; IN COMMON CASE MORE OPTIMAL SUCH COMPARISON FOR WHOLE STRUCTURE NOT PER FIELD
					\\\\}
 common

				;if $-\\..base-sizeof.#name			  ; IF WE REMOVE 3 LINES ABOVE COMMENTED AS LEGACY
				;	 err					  ; THEN WE MUST UNCOMMENT THESE 3 LINES
				;end if 					  ;
				label . \\\}
		\\}
		macro name value \\{
			\\local anonymous
			match , @struct \\\{ define field@struct -,name,<values> \\\}
			match no, @struct \\\{ anonymous name value \\\} \\} \} }
;==============================================================
macro define@struct name,[anon,field,type,def] {
 common
	local list
	list equ
 forward
	local anonsize
;!!! WE MAKING ASSUMPTION THAT UNNAMED FIELDS ARE JUST RESERVE SPACE
	anonsize = $
	match any, anon \{ irp anondef,any \\{ anondef \\} \}
	anonsize = $-anonsize
;    END OF ASSUMPTION
	if ~ field eq .
		name#field type def
		sizeof.#name#field = $ - name#field
	else
		label name#.#type
		rb sizeof.#type
	end if
	local value
	match any, list \{ list equ list, \}
	list equ list <value>
 common
	sizeof.#name = $
	restruc name
	match values, list \{
		struc name value \\{
			\\local \\..base
			match , @struct \\\{ define field@struct .,name,<values> \\\}
			match no, @struct \\\{
				label \\..base
 forward
;!!! WE MAKING ASSUMPTION THAT UNNAMED FIELDS ARE JUST RESERVE SPACE
				rb anonsize
;    END of ASSUMPTION
				match , value \\\\{ field type def \\\\}
				match any, value \\\\{
					field type value
					if ~ field eq . 			  ; MAIN PURPOSE OF THEESE 3 LINES - REALISATION OF LEGACY ALIGNMENT STYLE IN STRUCTURES
						rb sizeof.#name#field - ($-field) ; BECAUSE ALIGNED FIELD PRECIDED WITH ANOTHER SHORTER SIZE FIELD - WE CAN EMULATE ALIGNMENT BY ADDING SEQUENCE OF ",?" TO END OF THAT SHORTER SIZE FIELD
					end if					  ; 2ND PURPOSE - COMPARISON THAT DATA PASSED TO INITIALIZE STRUCTURE WILL FIT IN ITS MEMBERS; IN COMMON CASE MORE OPTIMAL SUCH COMPARISON FOR WHOLE STRUCTURE NOT PER FIELD
					\\\\}
 common

				;if $-\\..base-sizeof.#name			  ; IF WE REMOVE 3 LINES ABOVE COMMENTED AS LEGACY
				;	 err					  ; THEN WE MUST UNCOMMENT THESE 3 LINES
				;end if 					  ;
				label . at \\..base \\\}
		\\}
		macro name value \\{
			\\local anonymous
			match , @struct \\\{ define field@struct -,name,<values> \\\}
			match no, @struct \\\{ anonymous name value \\\} \\} \} }
;==============================================================
macro enable@substruct {
	macro make@substruct substruct,parent,name,[field,type,def] \{
	 \common
		\local CONVERTED,PREVIOUS,CURRENT
		define CONVERTED parent,name
		define CURRENT 1
	 \forward
		PREVIOUS equ CURRENT
		define CURRENT 1
		\local sub
		match , field \\{
			match any, type \\\{
				define CURRENT 2
				enable@substruct
				make@substruct type,parent,sub def
				purge make@substruct \\\} \\}
		match -, field \\{ define CURRENT 0 \\}
		match =1:=0, PREVIOUS:CURRENT \\{ CONVERTED equ CONVERTED, < <type def> 				     \\}
		match =0:=0, PREVIOUS:CURRENT \\{ CONVERTED equ CONVERTED	       , <type def>			     \\}
		match =1:=1, PREVIOUS:CURRENT \\{ CONVERTED equ CONVERTED,			     , .\#field, type, <def> \\}
		match =0:=1, PREVIOUS:CURRENT \\{ CONVERTED equ CONVERTED			    >, .\#field, type, <def> \\}
		match =1:=2, PREVIOUS:CURRENT \\{ CONVERTED equ CONVERTED,			     , .,	 sub,	     \\}
		match =0:=2, PREVIOUS:CURRENT \\{ CONVERTED equ CONVERTED			    >, .,	 sub,	     \\}
		match =2, CURRENT \\{ define CURRENT 1 \\}
	 \common
		\local anonymous
		match =0, CURRENT \\{ CONVERTED equ CONVERTED					    >,anonymous, :,	     \\}
		match converted, CONVERTED \\{ define@\#substruct converted \\} \} }
;==============================================================
enable@substruct
;==============================================================
macro define@subunion parent,name,[anon,field,type,def] {
 common
	virtual at parent#.#name
 forward
		match any, anon \{
			irp anondef,any \\{
				\\local anonsize
				virtual at 0
					anondef
					anonsize = $
				end virtual
				if anonsize > $ - parent#.#name
					rb anonsize - ($-$$)
				end if
				\\}  \}
		if ~ field eq .
			virtual at parent#.#name
				parent#field type def
				sizeof.#parent#field = $ - parent#field
			end virtual
			if sizeof.#parent#field > $ - parent#.#name
				rb sizeof.#parent#field - ($ - parent#.#name)
			end if
		else
			virtual at parent#.#name
				label parent#.#type
				type def
			end virtual
			label name#.#type at parent#.#name
			if sizeof.#type > $ - parent#.#name
				rb sizeof.#type - ($ - parent#.#name)
			end if
		end if
 common
		sizeof.#name = $ - parent#.#name
	end virtual
	struc name value \{
		label .\#name
		last@union equ
 forward
		match any, last@union \\{
			virtual at .\#name
				field type def
			end virtual \\}
		match , last@union \\{
			match , value \\\{ field type def \\\}
			match any, value \\\{ field type value \\\} \\}
		last@union equ field
 common
		rb sizeof.#name - ($ - .\#name) \}
	macro name value \{
		label .\#name
		last@union equ
 forward
		match , last@union \\{ match any, value \\\{ field type value \\\} \\}
		last@union equ field
 common
		rb sizeof.#name - ($ - .\#name) \} }
;==============================================================
macro define@subrstruct parent,name,[anon,field,type,def] {
 common
	local ..start,..size
	virtual at parent#.#name-..size
		..start:
 forward
		local value,anonsize
;!!! WE MAKING ASSUMPTION THAT UNNAMED FIELDS ARE JUST RESERVE SPACE
		anonsize = $
		match any, anon \{ irp anondef,any \\{ anondef \\} \}
		anonsize = $-anonsize
;    END OF ASSUMPTION
		if ~ field eq .
			parent#field type def
			sizeof.#parent#field = $ - parent#field
		else
			label parent#.#type
			rb sizeof.#type
		end if
 common
		..size = $ - ..start
		sizeof.#name = ..size
	end virtual
	struc name value \{
		label .\#name
 forward
;!!! WE MAKING ASSUMPTION THAT UNNAMED FIELDS ARE JUST RESERVE SPACE
		rb anonsize
;    END of ASSUMPTION
		match , value \\{ field type def \\}
		match any, value \\{
			field type value
			if ~ field eq . 			    ; MAIN PURPOSE OF THEESE 3 LINES - REALISATION OF LEGACY ALIGNMENT STYLE IN STRUCTURES
				rb sizeof.#parent#field - ($-field) ; BECAUSE ALIGNED FIELD PRECIDED WITH ANOTHER SHORTER SIZE FIELD - WE CAN EMULATE ALIGNMENT BY ADDING SEQUENCE OF ",?" TO END OF THAT SHORTER SIZE FIELD
			end if \\}				    ; 2ND PURPOSE - COMPARISON THAT DATA PASSED TO INITIALIZE STRUCTURE WILL FIT IN ITS MEMBERS; IN COMMON CASE MORE OPTIMAL SUCH COMPARISON FOR WHOLE STRUCTURE NOT PER FIELD
 common
				;if $-.\#name-sizeof.#name	    ; IF WE REMOVE 3 LINES ABOVE COMMENTED AS LEGACY
				;	 err			    ; THEN WE MUST UNCOMMENT THESE 3 LINES
				;end if 			    ;
	\}
	macro name value \{
		\local ..anonymous
		..anonymous name \} }
;==============================================================
macro define@substruct parent,name,[anon,field,type,def] {
 common
	virtual at parent#.#name
 forward
		local value,anonsize
;!!! WE MAKING ASSUMPTION THAT UNNAMED FIELDS ARE JUST RESERVE SPACE
		anonsize = $
		match any, anon \{ irp anondef,any \\{ anondef \\} \}
		anonsize = $-anonsize
;    END OF ASSUMPTION
		if ~ field eq .
			parent#field type def
			sizeof.#parent#field = $ - parent#field
		else
			label parent#.#type
			rb sizeof.#type
		end if
 common
		sizeof.#name = $ - parent#.#name
	end virtual
	struc name value \{
		label .\#name
 forward
;!!! WE MAKING ASSUMPTION THAT UNNAMED FIELDS ARE JUST RESERVE SPACE
		rb anonsize
;    END of ASSUMPTION
		match , value \\{ field type def \\}
		match any, value \\{
			field type value
			if ~ field eq . 			    ; MAIN PURPOSE OF THEESE 3 LINES - REALISATION OF LEGACY ALIGNMENT STYLE IN STRUCTURES
				rb sizeof.#parent#field - ($-field) ; BECAUSE ALIGNED FIELD PRECIDED WITH ANOTHER SHORTER SIZE FIELD - WE CAN EMULATE ALIGNMENT BY ADDING SEQUENCE OF ",?" TO END OF THAT SHORTER SIZE FIELD
			end if \\}				    ; 2ND PURPOSE - COMPARISON THAT DATA PASSED TO INITIALIZE STRUCTURE WILL FIT IN ITS MEMBERS; IN COMMON CASE MORE OPTIMAL SUCH COMPARISON FOR WHOLE STRUCTURE NOT PER FIELD
 common
				;if $-.\#name-sizeof.#name	    ; IF WE REMOVE 3 LINES ABOVE COMMENTED AS LEGACY
				;	 err			    ; THEN WE MUST UNCOMMENT THESE 3 LINES
				;end if 			    ;
	\}
	macro name value \{
		\local ..anonymous
		..anonymous name \} }