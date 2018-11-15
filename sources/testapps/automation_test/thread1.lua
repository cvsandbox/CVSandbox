a = 1

---[[
function Main()
    b = Host.GetVariable( "b" )
    print( "thread, a:", a, ", b:", b )
    a = a + 1
    Host.SetVariable( "a", a )
end
--]]