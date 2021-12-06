using LinearAlgebra, MLDatasets

function write_matrix_binary(A::AbstractMatrix{Float32}, filename::String)
	open(filename, "w") do file
		write(file, size(A,1)); # write an Int64
		write(file, size(A,2)); # write an Int64
		for i in eachindex(A)
			write( file, A[i] ); # write Float64 (column-major by default)
		end
	end
end

train_x, _ = MNIST.traindata();
test_x,  _ = MNIST.testdata();
X = Float32.( [reshape(train_x,28*28,:) reshape(test_x,28*28,:)] );
write_matrix_binary(X, "Desktop/mnist.bin");