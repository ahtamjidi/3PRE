%#codegen
function result = test_function(a)
for i =1:10000
    a=a+1;
end
result =a;
end