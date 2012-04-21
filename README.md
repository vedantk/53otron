This is an embeddable just-in-time compiler for math. 

I built it for fun at the recent Berkeley vs. Stanford hackathon, where it was pretty well received. 

It uses LLVM for code generation. Everything else was written from scratch.

### Vectorization

The compiler makes exclusive use of Intel's fast packed-struct (or [SIMD](http://en.wikipedia.org/wiki/Simd)) instructions.

This lets it generate code that is faster than what clang or gcc can currently produce, even at the highest optimization levels.

That's possible because the functions you write are converted to operate on *vectors*, or collections of data. These vectors are processed in 128-bit SSE registers, which can increase your per-instruction throughput by a factor of 4.

[muparser](muparser.sourceforge.net) is a similar project, but as far as I can tell, it only emits single-scalar instructions (like clang and gcc).

### Examples

	$ ./repl
	Herro! Dis simd-lisp repl.
	> 1
	<1, 1, 1, 1>
	> (def tan (x) (/ (sin x) (cos x)))
	> (tan (exp 3))
	<2.87427, 2.87427, 2.87427, 2.87427>

	$ ./repl -emit
	Herro! Dis simd-lisp repl.
	>
	Exiting.
	; ModuleID = 'jit'

	declare void @llvm.x86.sse.storeu.ps(i8*, <4 x float>) nounwind

	declare <4 x float> @llvm.sqrt.v4f32(<4 x float>) nounwind readonly

	declare <4 x float> @llvm.sin.v4f32(<4 x float>) nounwind readonly

	declare <4 x float> @llvm.cos.v4f32(<4 x float>) nounwind readonly

	declare <4 x float> @llvm.exp.v4f32(<4 x float>) nounwind readonly

	declare <4 x float> @llvm.log.v4f32(<4 x float>) nounwind readonly

	declare <4 x float> @llvm.pow.v4f32(<4 x float>, <4 x float>) nounwind readonly

	define void @magic(float*, float*, i8*) {
	magic:
	  %3 = bitcast float* %0 to <4 x float>*
	  %x = load <4 x float>* %3, align 16
	  %4 = bitcast float* %1 to <4 x float>*
	  %y = load <4 x float>* %4, align 16
	  %5 = call <4 x float> @llvm.sin.v4f32(<4 x float> %x)
	  %6 = call <4 x float> @llvm.cos.v4f32(<4 x float> %x)
	  %7 = fdiv <4 x float> %5, %6
	  %8 = call <4 x float> @llvm.pow.v4f32(<4 x float> %x, <4 x float> %y)
	  %9 = fmul <4 x float> %7, %8
	  call void @llvm.x86.sse.storeu.ps(i8* %2, <4 x float> %9)
	  ret void
	}
	Calling magic...
	x = <0.25, 0.75, 1.25, 1.75>
	y = <1, 2, 4.5, 11.5>
	Result = <0.0638355, 0.524023, 8.21485, -3442.76>

If you want to figure out how to embed this compiler, check out the 'magic' function at the end of [repl.cc](https://github.com/vedantk/53otron/blob/master/repl.cc).
