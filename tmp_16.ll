declare i32 @printf(i8*, ...)
declare void @exit(i32)
@.int_specifier = constant [4 x i8] c"%d\0A\00"
@.str_specifier = constant [4 x i8] c"%s\0A\00"

@var3 = constant [12 x i8] c"already? :(\00"

@var6 = constant [3 x i8] c"ok\00"

@var23 = constant [8 x i8] c"alright\00"

@var42 = constant [6 x i8] c"back!\00"

@var45 = constant [6 x i8] c"here!\00"

@var79 = constant [6 x i8] c"great\00"

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

define i32 @foo(i1, i32, i8) {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    ;Storing value in var x
    %var0 = getelementptr i32, i32* %frame_ptr, i32 0      
    store i32 %1, i32* %var0      
    br label %label_19      ;defalut jump at each statement's end to the next one
label_19:
    ;Storing value in var y
    %var1 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var2 = zext i8 %2  to i32      
    store i32 %var2, i32* %var1      
    br label %label_25      ;defalut jump at each statement's end to the next one
label_25:
    br i1 %0, label %label_35, label %label_27      
label_27:
    %var4 = getelementptr [12 x i8], [12 x i8]* @var3 , i32 0, i32 0      
    call void @print(i8* %var4)      
    br label %label_31      ;defalut jump at each statement's end to the next one
label_31:
    %var5 = add i32 15, 0      
    ret i32 %var5      
    br label %label_35      ;defalut jump at each statement's end to the next one
label_35:
    %var7 = getelementptr [3 x i8], [3 x i8]* @var6 , i32 0, i32 0      
    call void @print(i8* %var7)      
    br label %label_39      ;defalut jump at each statement's end to the next one
label_39:
    br label %label_41      ;Dummy branch to support while
label_41:
    ;Getting var x
    %var8 = getelementptr i32, i32* %frame_ptr, i32 0      
    %var9 = load i32, i32* %var8      
    %var10 = add i32 10, 0      
    %var11 = icmp slt i32 %var9, %var10      
    br i1 %var11, label %label_48, label %label_83      
label_48:
    ;Getting var x
    %var12 = getelementptr i32, i32* %frame_ptr, i32 0      
    %var13 = load i32, i32* %var12      
    %var14 = add i32 1, 0      
    %var15 = add i32 %var13, %var14      
    ;Storing value in var x
    %var16 = getelementptr i32, i32* %frame_ptr, i32 0      
    store i32 %var15, i32* %var16      
    br label %label_58      ;defalut jump at each statement's end to the next one
label_58:
    br i1 %0, label %label_61, label %label_41      
    br label %label_61      ;defalut jump at each statement's end to the next one
label_61:
    ;Getting var x
    %var17 = getelementptr i32, i32* %frame_ptr, i32 0      
    %var18 = load i32, i32* %var17      
    %var19 = add i32 7, 0      
    %var20 = icmp eq i32 %var18, %var19      
    br i1 %var20, label %label_68, label %label_41      
label_68:
    ;Getting var x
    %var21 = getelementptr i32, i32* %frame_ptr, i32 0      
    %var22 = load i32, i32* %var21      
    call void @printi(i32 %var22)      
    br label %label_74      ;defalut jump at each statement's end to the next one
label_74:
    %var24 = getelementptr [8 x i8], [8 x i8]* @var23 , i32 0, i32 0      
    call void @print(i8* %var24)      
    br label %label_78      ;defalut jump at each statement's end to the next one
label_78:
    %var25 = add i32 17, 0      
    ret i32 %var25      
    br label %label_41      ;jump to while condition
    br label %label_83      ;defalut jump at each statement's end to the next one
label_83:
    br label %label_85      ;Dummy branch to support while
label_85:
    ;Getting var y
    %var26 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var27 = load i32, i32* %var26      
    %var28 = trunc i32 %var27 to i8      
    %var29 = add i8 12, 0      
    %var30 = icmp ugt i8 %var28, %var29      
    br i1 %var30, label %label_93, label %label_122      
label_93:
    ;Getting var y
    %var31 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var32 = load i32, i32* %var31      
    %var33 = trunc i32 %var32 to i8      
    %var34 = add i8 1, 0      
    %var35 = sub i8 %var33, %var34      
    ;Storing value in var y
    %var36 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var37 = zext i8 %var35  to i32      
    store i32 %var37, i32* %var36      
    br label %label_105      ;defalut jump at each statement's end to the next one
label_105:
    ;Getting var y
    %var38 = getelementptr i32, i32* %frame_ptr, i32 1      
    %var39 = load i32, i32* %var38      
    %var40 = trunc i32 %var39 to i8      
    %var41 = zext i8 %var40  to i32      
    call void @printi(i32 %var41)      
    br label %label_113      ;defalut jump at each statement's end to the next one
label_113:
    %var43 = getelementptr [6 x i8], [6 x i8]* @var42 , i32 0, i32 0      
    call void @print(i8* %var43)      
    br label %label_117      ;defalut jump at each statement's end to the next one
label_117:
    %var44 = add i32 18, 0      
    ret i32 %var44      
    br label %label_85      ;jump to while condition
    br label %label_122      ;defalut jump at each statement's end to the next one
label_122:
    %var46 = getelementptr [6 x i8], [6 x i8]* @var45 , i32 0, i32 0      
    call void @print(i8* %var46)      
    br label %label_126      ;defalut jump at each statement's end to the next one
label_126:
    %var47 = add i32 322, 0      
    ret i32 %var47      
}
      
define i1 @opposite(i1) {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    br label %label_134      ;Dummy branch to support while
label_134:
    %var48 = add i1 1, 0      
    br i1 %var48, label %label_137, label %label_176      
label_137:
    br label %label_139      ;Dummy branch to support while
label_139:
    %var49 = add i1 1, 0      
    br i1 %var49, label %label_142, label %label_134      
label_142:
    br label %label_144      ;Dummy branch to support while
label_144:
    %var50 = add i1 1, 0      
    br i1 %var50, label %label_147, label %label_139      
label_147:
    br label %label_149      ;Dummy branch to support while
label_149:
    %var51 = add i1 1, 0      
    br i1 %var51, label %label_152, label %label_144      
label_152:
    br label %label_154      ;Dummy branch to support while
label_154:
    %var52 = add i1 1, 0      
    br i1 %var52, label %label_157, label %label_168      
label_157:
    br i1 %0, label %label_161, label %label_159      
label_159:
    br label %label_163      
label_161:
    br label %label_163      
label_163:
    %var53 = phi i1 [1, %label_159], [0, %label_161]      
    ret i1 %var53      
    br label %label_154      ;jump to while condition
    br label %label_168      ;defalut jump at each statement's end to the next one
label_168:
    ;Jumping to statement after while(found break)
    br label %label_144      
    br label %label_149      ;jump to while condition
    br label %label_144      ;jump to while condition
    br label %label_139      ;jump to while condition
    br label %label_134      ;jump to while condition
    br label %label_176      ;defalut jump at each statement's end to the next one
label_176:
    br i1 %0, label %label_180, label %label_178      
label_178:
    br label %label_182      
label_180:
    br label %label_182      
label_182:
    %var54 = phi i1 [1, %label_178], [0, %label_180]      
    ret i1 %var54      
}
      
define i8 @multiply(i8, i8) {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var55 = mul i8 %0, %1      
    ret i8 %var55      
}
      
define void @main() {      
    %frame_ptr = alloca i32, i32 50      ;Allocating local variables
    
    %var56 = add i1 0, 0      
    br i1 %var56, label %label_203, label %label_205      
    br label %label_198      ;jump next exp in expList
label_198:
    %var57 = add i32 5, 0      
    br label %label_201      ;jump next exp in expList
label_201:
    %var58 = add i8 15, 0      
label_203:
    br label %label_207      
label_205:
    br label %label_207      
label_207:
    %var59 = phi i1 [1, %label_203], [0, %label_205]      
    br label %label_198      
    %var60 = call i32 @foo(i1 %var59, i32 %var57, i8 %var58)      
    call void @printi(i32 %var60)      
    br label %label_213      ;defalut jump at each statement's end to the next one
label_213:
    %var61 = add i1 1, 0      
    br i1 %var61, label %label_222, label %label_224      
    br label %label_217      ;jump next exp in expList
label_217:
    %var62 = add i32 5, 0      
    br label %label_220      ;jump next exp in expList
label_220:
    %var63 = add i8 21, 0      
label_222:
    br label %label_226      
label_224:
    br label %label_226      
label_226:
    %var64 = phi i1 [1, %label_222], [0, %label_224]      
    br label %label_217      
    %var65 = call i32 @foo(i1 %var64, i32 %var62, i8 %var63)      
    call void @printi(i32 %var65)      
    br label %label_232      ;defalut jump at each statement's end to the next one
label_232:
    %var66 = add i1 1, 0      
    br i1 %var66, label %label_241, label %label_243      
    br label %label_236      ;jump next exp in expList
label_236:
    %var67 = add i32 11, 0      
    br label %label_239      ;jump next exp in expList
label_239:
    %var68 = add i8 4, 0      
label_241:
    br label %label_245      
label_243:
    br label %label_245      
label_245:
    %var69 = phi i1 [1, %label_241], [0, %label_243]      
    br label %label_236      
    %var70 = call i32 @foo(i1 %var69, i32 %var67, i8 %var68)      
    call void @printi(i32 %var70)      
    br label %label_251      ;defalut jump at each statement's end to the next one
label_251:
    %var71 = add i1 1, 0      
    br i1 %var71, label %label_260, label %label_262      
    br label %label_255      ;jump next exp in expList
label_255:
    %var72 = add i32 10, 0      
    br label %label_258      ;jump next exp in expList
label_258:
    %var73 = add i8 13, 0      
label_260:
    br label %label_264      
label_262:
    br label %label_264      
label_264:
    %var74 = phi i1 [1, %label_260], [0, %label_262]      
    br label %label_255      
    %var75 = call i32 @foo(i1 %var74, i32 %var72, i8 %var73)      
    call void @printi(i32 %var75)      
    br label %label_270      ;defalut jump at each statement's end to the next one
label_270:
    %var76 = add i1 0, 0      
    br i1 %var76, label %label_273, label %label_275      
label_273:
    br label %label_277      
label_275:
    br label %label_277      
label_277:
    %var77 = phi i1 [1, %label_273], [0, %label_275]      
    br label %label_280      
label_280:
    %var78 = call i1 @opposite(i1 %var77)      
    br i1 %var78, label %label_283, label %label_287      
label_283:
    %var80 = getelementptr [6 x i8], [6 x i8]* @var79 , i32 0, i32 0      
    call void @print(i8* %var80)      
    br label %label_287      ;defalut jump at each statement's end to the next one
label_287:
    %var81 = add i8 16, 0      
    br label %label_290      ;jump next exp in expList
label_290:
    %var82 = add i8 16, 0      
    %var83 = call i8 @multiply(i8 %var81, i8 %var82)      
    %var84 = zext i8 %var83  to i32      
    call void @printi(i32 %var84)      
    br label %label_296      
label_296:
    ret void      
}
      
