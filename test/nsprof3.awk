function a() { print "awk::a" }
function b() { print "awk::b" }

@namespace "foo"

function a() { awk::a() }
function b() { awk::b() }

@namespace "bar"

function a() { awk::a() }
function b() { awk::b() }