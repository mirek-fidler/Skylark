function prepareData() {
	
  var login = (document.getElementById("lo").value).toLowerCase();
  var password = document.getElementById("pa").value
  if(password.length > 0 && login.length > 0){
		document.getElementById("lo").style.visibility = 'hidden';
		document.getElementById("pa").style.visibility = 'hidden';
		
		document.getElementById("lo").value = login;
		password = hex_sha256( password + "=" + login);
		document.getElementById("pa").value = password;
		return true;
  }
  alert("You must fill both input before doing magic !");
  return false;
}