	;; nasm macro
	%include '../c32.mac'
SECTION .text

	;; global definition
	global do_inner_doublex_i386

	
	proc do_inner_doublex_i386
	%$dst   arg
	%$src   arg
	%$sw	arg
	%$sh	arg
	%$sx arg

	push edi
	push esi
	push eax
	push ebx
	push edx
	
	mov edi,[ebp + %$dst]	; edi=pointer on dest buffer
	mov esi,[ebp + %$src]	; esi=pointer on src buffer
	mov edx,[ebp + %$sx]
	shl edx,2
		;; 	movq mm2, [scan_mask16]
		;; 	punpckldq mm2,mm2

	
	mov eax,[ebp +	%$sh]	; loop counter for y
loop_x:	
	mov ebx,[ebp +	%$sw]	; loop counter for x ((w/2)/8)
	shr ebx,3
loop_y:


	movd mm0,[esi]
	movd mm1,[esi+4]
	
	movd mm2,[esi+8]
	movd mm3,[esi+12]
	
	movd mm4,[esi+16]
	movd mm5,[esi+20]
	
	movd mm6,[esi+24]
	movd mm7,[esi+28]
	
	punpcklwd mm0,mm0
	punpcklwd mm1,mm1
	punpcklwd mm2,mm2
	punpcklwd mm3,mm3
	punpcklwd mm4,mm4
	punpcklwd mm5,mm5
	punpcklwd mm6,mm6
	punpcklwd mm7,mm7
	
	movq [edi],mm0
	movq [edi+8],mm1
	movq [edi+16],mm2
	movq [edi+24],mm3

	movq [edi+32],mm4
	movq [edi+40],mm5
	movq [edi+48],mm6
	movq [edi+56],mm7
	
	add esi,32
	add edi,64
	
	dec ebx
	cmp ebx,0
	jne loop_y

 	add esi,edx
		;; 	add edi,32

	
	dec eax
	cmp eax,0
	jne loop_x


	pop ebx
	pop eax
	pop esi
	pop edi

	emms
	
	endproc
