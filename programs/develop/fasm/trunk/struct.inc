
; Macroinstructions for defining data structures

macro struct name
 { fields@struct equ name
   match child parent, name \{ fields@struct equ child,fields@\#parent \}
   sub@struct equ
   struc db [val] \{ \common fields@struct equ fields@struct,.,db,<val> \}
   struc dw [val] \{ \common fields@struct equ fields@struct,.,dw,<val> \}
   struc du [val] \{ \common fields@struct equ fields@struct,.,du,<val> \}
   struc dd [val] \{ \common fields@struct equ fields@struct,.,dd,<val> \}
   struc dp [val] \{ \common fields@struct equ fields@struct,.,dp,<val> \}
   struc dq [val] \{ \common fields@struct equ fields@struct,.,dq,<val> \}
   struc dt [val] \{ \common fields@struct equ fields@struct,.,dt,<val> \}
   struc rb count \{ fields@struct equ fields@struct,.,db,count dup (?) \}
   struc rw count \{ fields@struct equ fields@struct,.,dw,count dup (?) \}
   struc rd count \{ fields@struct equ fields@struct,.,dd,count dup (?) \}
   struc rp count \{ fields@struct equ fields@struct,.,dp,count dup (?) \}
   struc rq count \{ fields@struct equ fields@struct,.,dq,count dup (?) \}
   struc rt count \{ fields@struct equ fields@struct,.,dt,count dup (?) \}
   macro db [val] \{ \common \local anonymous
		     fields@struct equ fields@struct,anonymous,db,<val> \}
   macro dw [val] \{ \common \local anonymous
		     fields@struct equ fields@struct,anonymous,dw,<val> \}
   macro du [val] \{ \common \local anonymous
		     fields@struct equ fields@struct,anonymous,du,<val> \}
   macro dd [val] \{ \common \local anonymous
		     fields@struct equ fields@struct,anonymous,dd,<val> \}
   macro dp [val] \{ \common \local anonymous
		     fields@struct equ fields@struct,anonymous,dp,<val> \}
   macro dq [val] \{ \common \local anonymous
		     fields@struct equ fields@struct,anonymous,dq,<val> \}
   macro dt [val] \{ \common \local anonymous
		     fields@struct equ fields@struct,anonymous,dt,<val> \}
   macro rb count \{ \local anonymous
		     fields@struct equ fields@struct,anonymous,db,count dup (?) \}
   macro rw count \{ \local anonymous
		     fields@struct equ fields@struct,anonymous,dw,count dup (?) \}
   macro rd count \{ \local anonymous
		     fields@struct equ fields@struct,anonymous,dd,count dup (?) \}
   macro rp count \{ \local anonymous
		     fields@struct equ fields@struct,anonymous,dp,count dup (?) \}
   macro rq count \{ \local anonymous
		     fields@struct equ fields@struct,anonymous,dq,count dup (?) \}
   macro rt count \{ \local anonymous
		     fields@struct equ fields@struct,anonymous,dt,count dup (?) \}
   macro union \{ fields@struct equ fields@struct,,union,<
		  sub@struct equ union \}
   macro struct \{ fields@struct equ fields@struct,,substruct,<
		  sub@struct equ substruct \}
   virtual at 0 }

macro ends
 { match , sub@struct \{ restruc db,dw,du,dd,dp,dq,dt
			 restruc rb,rw,rd,rp,rq,rt
			 purge db,dw,du,dd,dp,dq,dt
			 purge rb,rw,rd,rp,rq,rt
			 purge union,struct
			 match name=,fields,fields@struct \\{ fields@struct equ
							      make@struct name,fields
							      fields@\\#name equ fields \\}
			 end virtual \}
   match any, sub@struct \{ fields@struct equ fields@struct> \}
   restore sub@struct }

macro make@struct name,[field,type,def]
 { common
    if $
     display 'Error: definition of ',`name,' contains illegal instructions.',0Dh,0Ah
     err
    end if
    local define
    define equ name
   forward
    local sub
    match , field \{ make@substruct type,name,sub def
		     define equ define,.,sub, \}
    match any, field \{ define equ define,.#field,type,<def> \}
   common
    match fields, define \{ define@struct fields \} }

macro define@struct name,[field,type,def]
 { common
    local list
    list equ
   forward
    if ~ field eq .
     name#field type def
     sizeof.#name#field = $ - name#field
    else
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
    match any, fields@struct \\\{ fields@struct equ fields@struct,.,name,<values> \\\}
    match , fields@struct \\\{ label .
   forward
     match , value \\\\{ field type def \\\\}
     match any, value \\\\{ field type value
			   if ~ field eq .
			    rb sizeof.#name#field - ($-field)
			   end if \\\\}
   common \\\} \\} \} }

macro enable@substruct
 { macro make@substruct substruct,parent,name,[field,type,def]
    \{ \common
	\local define
	define equ parent,name
       \forward
	\local sub
	match , field \\{ match any, type \\\{ enable@substruct
					       make@substruct type,name,sub def
					       purge make@substruct
					       define equ define,.,sub, \\\} \\}
	match any, field \\{ define equ define,.\#field,type,<def> \\}
       \common
	match fields, define \\{ define@\#substruct fields \\} \} }

enable@substruct

macro define@union parent,name,[field,type,def]
 { common
    virtual at 0
   forward
    if ~ field eq .
     virtual at 0
      parent#field type def
      sizeof.#parent#field = $ - parent#field
     end virtual
     if sizeof.#parent#field > $
      rb sizeof.#parent#field - $
     end if
    else if sizeof.#type > $
     rb sizeof.#type - $
    end if
   common
    sizeof.#name = $
    end virtual
    struc name [value] \{ \common
    label .\#name
    last@union equ
   forward
    match any, last@union \\{ virtual at .\#name
			       field type def
			      end virtual \\}
    match , last@union \\{ match , value \\\{ field type def \\\}
			   match any, value \\\{ field type value \\\} \\}
    last@union equ field
   common rb sizeof.#name - ($ - .\#name) \} }

macro define@substruct parent,name,[field,type,def]
 { common
    virtual at 0
   forward
    if ~ field eq .
     parent#field type def
     sizeof.#parent#field = $ - parent#field
    else
     rb sizeof.#type
    end if
    local value
   common
    sizeof.#name = $
    end virtual
    struc name value \{
    label .\#name
   forward
     match , value \\{ field type def \\}
     match any, value \\{ field type value
			  if ~ field eq .
			   rb sizeof.#parent#field - ($-field)
			  end if \\}
   common \} }
