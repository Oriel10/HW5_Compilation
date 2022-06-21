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

define void @maxTestsBasicAssignmentTest() {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    ;Initializing var x to 0
    %var0 = getelementptr i32, i32* %frame_ptr, i32 0      
    store i32 0, i32* %var0      
    br label %label_19      ;defalut jump at each statement's end to the next one
label_19:
    ;Initializing var y to 0
    %var1 = getelementptr i32, i32* %frame_ptr, i32 1      
    store i32 0, i32* %var1      
    br label %label_24      ;defalut jump at each statement's end to the next one
label_24:
    ;Getting var x
    %var2 = getelementptr i32, i32* %frame_ptr, i32 0      
    %var3 = load i32, i32* %var2      
    call void @printi(i32 %var3)      
    br label %label_30      ;defalut jump at each statement's end to the next one
label_30:
    ;Getting var y
    %var4 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var5 = load i32, i32* %var4      
    call void @printi(i32 %var5)      
    br label %label_36      
label_36:
    ret void      
}
      
define void @main() {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    call void @maxTestsBasicAssignmentTest()      
    br label %label_44      
label_44:
    ret void      
}
      
