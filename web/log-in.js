name = "petar";
pass = "aliluja";

function validateForm()
{
    docName = document.forms['reg']['username'].value; 
    docPass = document.forms['reg']['password'].value;
    if(docName != name || docPass != pass)
        return false;
    else
        return true;
}