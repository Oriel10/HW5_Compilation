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
    br label %label_20      
label_20:
    ;Getting var x
    %var2 = getelementptr i32, i32* %frame_ptr, i32 0      
    %var3 = load i32, i32* %var2      
    ret i32 %var3      
    br label @      
    ret i32 0      
}
      
define void @main() {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var4 = add i32 3, 0      
    %var5 = add i32 2, 0      
    %var6 = icmp sgt i32 %var4, %var5      
    br i1 %var6, label %label_35, label %label_42      
label_35:
    %var7 = add i8 3, 0      
    ;Storing value in var x2
    %var8 = getelementptr i32, i32* %frame_ptr, i32 0      
    %var9 = zext i8 %var7 to i32      
    store i32 %var9, i32* %var8      
    br label %label_42      
label_42:
    %var10 = add i32 3, 0      
    %var11 = add i32 2, 0      
    %var12 = icmp sgt i32 %var10, %var11      
    ;Storing value in var x
    %var13 = getelementptr i32, i32* %frame_ptr, i32 0      
    store i32 %failedCasting, i32* %var13      
    br label @      
    ret void      
}
      
