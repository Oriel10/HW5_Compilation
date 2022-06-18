declare i32 @printf(i8*, ...)
declare void @exit(i32)
@.int_specifier = constant [4 x i8] c"%d\0A\00"
@.str_specifier = constant [4 x i8] c"%s\0A\00"

define void @printi(i32) {      
    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0      
    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)      
    ret void      
}      

define void @print(i8*) {      
    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0      
    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)      
    ret void      
}      

define i32 @goo() {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var0 = add i32 2, 0      
    ;Storing value in var x
    %var1 = getelementptr i32, i32* %frame_ptr, i32 0      
    store i32 %var0, i32* %var1      
    br label @      
    ret i32 0      
}
      
define i8 @foo(i8, i8) {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    ;Initializing var z to 0
    %var2 = getelementptr i32, i32* %frame_ptr, i32 0      
    store i32 0, i32* %var2      
    br label %label_29      
label_29:
    %var3 = add i8 %0, %1      
    ;Storing value in var z
    %var4 = getelementptr i32, i32* %frame_ptr, i32 0      
    %var5 = zext i8 %var3 to i32      
    store i32 %var5, i32* %var4      
    br label %label_36      
label_36:
    ;Storing value in var w
    %var6 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var7 = zext i8 %0 to i32      
    store i32 %var7, i32* %var6      
    br label %label_42      
label_42:
    ;Getting var w
    %var8 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var9 = load i32, i32* %var8      
    %var10 = trunc i32 %var9 to i8      
    ;Storing value in var a
    %var11 = getelementptr i32, i32* %frame_ptr, i32 2      
    %var12 = zext i8 %var10 to i32      
    store i32 %var12, i32* %var11      
    br label @      
    ret i8 0      
}
      
define void @main() {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var13 = add i32 0, 0      
    ;Storing value in var h
    %var14 = getelementptr i32, i32* %frame_ptr, i32 0      
    store i32 %var13, i32* %var14      
    br label %label_62      
label_62:
    %var15 = add i8 2, 0      
    ;Storing value in var x
    %var16 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var17 = zext i8 %var15 to i32      
    store i32 %var17, i32* %var16      
    br label %label_69      
label_69:
    %var18 = add i32 3, 0      
    %var19 = add i32 2, 0      
    %var20 = icmp sgt i32 %var18, %var19      
    br i1 %var20, label %label_74, label %label_81      
label_74:
    %var21 = add i8 3, 0      
    ;Storing value in var x
    %var22 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var23 = zext i8 %var21 to i32      
    store i32 %var23, i32* %var22      
    br label %label_81      
label_81:
    %var24 = add i8 200, 0      
    ;Storing value in var id
    %var25 = getelementptr i32, i32* %frame_ptr, i32 2      
    %var26 = zext i8 %var24 to i32      
    store i32 %var26, i32* %var25      
    br label %label_88      
label_88:
    ;Initializing var f to 0
    %var27 = getelementptr i32, i32* %frame_ptr, i32 3      
    store i32 0, i32* %var27      
    br label %label_93      
label_93:
    ;Getting var id
    %var28 = getelementptr i32, i32* %frame_ptr, i32 2      
    %var29 = load i32, i32* %var28      
    %var30 = trunc i32 %var29 to i8      
    %var31 = zext i8 %var30 to i32      
    ;Storing value in var f
    %var32 = getelementptr i32, i32* %frame_ptr, i32 3      
    store i32 %var31, i32* %var32      
    br label %label_103      
label_103:
    ;Getting var f
    %var33 = getelementptr i32, i32* %frame_ptr, i32 3      
    %var34 = load i32, i32* %var33      
    call void @printi(i32 %var34)      
    br label %label_109      
label_109:
    ret void      
    br label @      
}
      
