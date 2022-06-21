declare i32 @printf(i8*, ...)
declare void @exit(i32)
@.int_specifier = constant [4 x i8] c"%d\0A\00"
@.str_specifier = constant [4 x i8] c"%s\0A\00"

@var0 = constant [4 x i8] c"f1!\00"

@var2 = constant [4 x i8] c"f2!\00"

@var4 = constant [4 x i8] c"f3!\00"

@var7 = constant [14 x i8] c"call me maybe\00"

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

define i32 @f1(i32) {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var1 = getelementptr [4 x i8], [4 x i8]* @var0 , i32 0, i32 0      
    call void @print(i8* %var1)      
    br label %label_18      ;defalut jump at each statement's end to the next one
label_18:
    ret i32 %0      
}
      
define i8 @f2(i8) {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var3 = getelementptr [4 x i8], [4 x i8]* @var2 , i32 0, i32 0      
    call void @print(i8* %var3)      
    br label %label_27      ;defalut jump at each statement's end to the next one
label_27:
    ret i8 %0      
}
      
define i1 @f3(i1) {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var5 = getelementptr [4 x i8], [4 x i8]* @var4 , i32 0, i32 0      
    call void @print(i8* %var5)      
    br label %label_36      ;defalut jump at each statement's end to the next one
label_36:
    br i1 %0, label %label_38, label %label_40      
label_38:
    br label %label_42      
label_40:
    br label %label_42      
label_42:
    %var6 = phi i1 [1, %label_38], [0, %label_40]      
    ret i1 %var6      
}
      
define void @callMe(i32, i32, i8, i1, i1, i8, i32, i1) {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var8 = getelementptr [14 x i8], [14 x i8]* @var7 , i32 0, i32 0      
    call void @print(i8* %var8)      
    br label %label_52      ;defalut jump at each statement's end to the next one
label_52:
    call void @printi(i32 %0)      
    br label %label_55      ;defalut jump at each statement's end to the next one
label_55:
    call void @printi(i32 %1)      
    br label %label_58      ;defalut jump at each statement's end to the next one
label_58:
    %var9 = zext i8 %2  to i32      
    call void @printi(i32 %var9)      
    br label %label_62      ;defalut jump at each statement's end to the next one
label_62:
    %var10 = zext i8 %5  to i32      
    call void @printi(i32 %var10)      
    br label %label_66      ;defalut jump at each statement's end to the next one
label_66:
    call void @printi(i32 %6)      
    br label %label_69      
label_69:
    ret void      
}
      
define void @main() {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var11 = add i32 2, 0      
    %var12 = call i32 @f1(i32 %var11)      
    br label %label_78      ;jump next exp in expList
label_78:
    %var13 = add i8 4, 0      
    %var14 = call i8 @f2(i8 %var13)      
    br label %label_82      ;jump next exp in expList
label_82:
    %var15 = add i8 5, 0      
    %var16 = call i8 @f2(i8 %var15)      
    br label %label_86      ;jump next exp in expList
label_86:
    %var17 = add i1 1, 0      
    br i1 %var17, label %label_89, label %label_91      
label_89:
    br label %label_93      
label_91:
    br label %label_93      
label_93:
    %var18 = phi i1 [1, %label_89], [0, %label_91]      
    br label %label_96      
label_96:
    %var19 = call i1 @f3(i1 %var18)      
    br i1 %var19, label %label_136, label %label_138      
    br label %label_100      ;jump next exp in expList
label_100:
    %var20 = add i1 0, 0      
    br i1 %var20, label %label_103, label %label_105      
label_103:
    br label %label_107      
label_105:
    br label %label_107      
label_107:
    %var21 = phi i1 [1, %label_103], [0, %label_105]      
    br label %label_110      
label_110:
    %var22 = call i1 @f3(i1 %var21)      
    br i1 %var22, label %label_143, label %label_145      
    br label %label_114      ;jump next exp in expList
label_114:
    %var23 = add i8 3, 0      
    %var24 = call i8 @f2(i8 %var23)      
    br label %label_118      ;jump next exp in expList
label_118:
    %var25 = add i32 22, 0      
    %var26 = call i32 @f1(i32 %var25)      
    br label %label_122      ;jump next exp in expList
label_122:
    %var27 = add i1 0, 0      
    br i1 %var27, label %label_125, label %label_127      
label_125:
    br label %label_129      
label_127:
    br label %label_129      
label_129:
    %var28 = phi i1 [1, %label_125], [0, %label_127]      
    br label %label_132      
label_132:
    %var29 = call i1 @f3(i1 %var28)      
    br i1 %var29, label %label_150, label %label_152 
    
         
    %var30 = zext i8 %var14  to i32      
label_136:
    br label %label_140      
label_138:
    br label %label_140      
label_140:
    %var31 = phi i1 [1, %label_136], [0, %label_138]      
    br label %label_100      
label_143:
    br label %label_147      
label_145:
    br label %label_147      
label_147:
    %var32 = phi i1 [1, %label_143], [0, %label_145]      
    br label %label_114      
label_150:
    br label %label_154      
label_152:
    br label %label_154      
label_154:
    %var33 = phi i1 [1, %label_150], [0, %label_152]      
    br label %label_157      
label_157:
    call void @callMe(i32 %var12, i32 %var30, i8 %var16, i1 %var31, i1 %var32, i8 %var24, i32 %var26, i1 %var33)      
    br label %label_160      
label_160:
    ret void      
}
      
