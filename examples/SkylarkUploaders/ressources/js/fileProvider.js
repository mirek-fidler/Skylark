function  AddUser(request){
	var login = (document.getElementById("lo").value).toLowerCase();
	var password = document.getElementById("pa").value

	if(password.length > 0 && login.length > 0){
		document.getElementById("loader").style.display = "block";
		password = hex_sha256( password + "=" + login);

		var xmlhttp = new XMLHttpRequest();
		xmlhttp.onreadystatechange = function() {
			if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
				var data = xmlhttp.responseText.substring(2,xmlhttp.responseText.length );
				const obj = JSON.parse(data);
				if(obj.result == 1){
					document.getElementById("loader").style.display = "none";
					document.getElementById("result").innerHTML = "Done";
					document.getElementById("result").style.display = "block";
					setTimeout(function(){ document.getElementById("result").style.display = "none";document.getElementById("result").innerHTML =""; }, 3000);
					document.getElementById("lo").value = "";
					document.getElementById("pa").value = "";
				}else{
					document.getElementById("loader").style.display = "none";
					document.getElementById("result").innerHTML = "Error";
					document.getElementById("result").style.display = "block";
					alert("Server say : " + obj.alert);
					setTimeout(function(){ document.getElementById("result").style.display = "none";document.getElementById("result").innerHTML =""; }, 3000);
				}
			}
		}
		var parameters = { val: "" };
		parameters.val += "login=" + login +"&pass="+ password +"&";
		
		if(__js_identity__ != undefined) {
			if(parameters.val.length)
				parameters.val += "&";
			parameters.val += "__js_identity__=" + __js_identity__;
		}
		xmlhttp.open("POST", request, true);
		xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
		xmlhttp.send(parameters.val);
	}else{
		alert("You must fill both input before adding new user !");
	}
}
function ChangePass(request){
	var oldPass = (document.getElementById("olpa").value);
	var newPass1 = (document.getElementById("pa2").value);
	var newPass2 = (document.getElementById("repa").value);
	var login = (document.getElementById("login").value).toLowerCase();
	if(oldPass.length > 0 && newPass1.length > 0 && newPass2.length > 0){
		if( newPass1 == newPass2){
			document.getElementById("loader2").style.display = "block";
			oldPass = hex_sha256( oldPass + "=" + login);
			newPass1 = hex_sha256( newPass1 + "=" + login);
			
			var xmlhttp = new XMLHttpRequest();
			xmlhttp.onreadystatechange = function() {
				if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
					var data = xmlhttp.responseText.substring(2,xmlhttp.responseText.length );
					const obj = JSON.parse(data);
					if(obj.result == 1){
						document.getElementById("loader2").style.display = "none";
						document.getElementById("result2").innerHTML = "Done";
						document.getElementById("result2").style.display = "block";
						setTimeout(function(){ document.getElementById("result").style.display = "none";document.getElementById("result").innerHTML =""; }, 3000);
						document.getElementById("olpa").value = "";
						document.getElementById("pa2").value = "";
						document.getElementById("repa").value = "";
					}else{
						document.getElementById("loader2").style.display = "none";
						document.getElementById("result2").innerHTML = "Error";
						document.getElementById("result2").style.display = "block";
						
						document.getElementById("olpa").value = "";
						document.getElementById("pa2").value = "";
						document.getElementById("repa").value = "";
						alert("Server say : " + obj.alert);
						setTimeout(function(){ document.getElementById("result").style.display = "none";document.getElementById("result").innerHTML =""; }, 3000);
					}
				}
			}
			var parameters = { val: "" };
			parameters.val += "oldpass=" + oldPass +"&newpass="+ newPass1 +"&";
			
			if(__js_identity__ != undefined) {
				if(parameters.val.length)
					parameters.val += "&";
				parameters.val += "__js_identity__=" + __js_identity__;
			}
			xmlhttp.open("POST", request, true);
			xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
			xmlhttp.send(parameters.val);
		}else{
			alert("Both new password are not identical !");
		}
	}else{
		alert("You must All input before adding new user !");
	}
}

// toggles progress bar visibility
function toggleBarVisibility() {
	var e = document.getElementById("progress_container");
	e.style.display = (e.style.display == "block") ? "none" : "block";
}
/*
// create http request object for upload
// browser dependent -- probably NOT completely cross browser
// if you use JQuery there are better ways
function createRequestObject() {
	var http;
	if (navigator.appName == "Microsoft Internet Explorer") {
		http = new ActiveXObject("Microsoft.XMLHTTP");
	}
	else {
		http = new XMLHttpRequest();
	}
	return http;
}


function sendRequest() {
	var http = createRequestObject();
	
	// use session cookie and current time to build an unique upload identifier
	var session = document.getElementById('session').value;
	var time = document.getElementById('submittime').value;
	
	// get upload url
	var url = document.getElementById('progressurl').value;
	
	// build GET uri and do the request
	http.open("GET", url + "?uploadid=" + session + time);
	
	// setup a callback handling end of request
	http.onreadystatechange = function () { handleResponse(http); };
	http.send(null);
}

// this callback handles 'progress' http requests results
function handleResponse(http) {
	var response;
	if (http.readyState == 4) {
		response = http.responseText;
		document.getElementById("progress").style.width = response + "%";
		document.getElementById("progress").innerHTML = response + "%";
		
		if (response < 100) {
			// if upload still not complete, repost a progress request after some time
			// (here 200 msec)
			setTimeout("sendRequest()", 200);
		}
		else {
			// if progress is complete, put 'Done' inside bar, then
			// run a callback that hides it after some time
			document.getElementById("progress").innerHTML = "100% Done.";
			// hides 'Done' status
			setTimeout("closeProgress()", 3000);
			AskForResult();
		}
	}
}*/


/*
// this callback hides status bar after some time upload is over
function closeProgress() {
	toggleBarVisibility();
	document.getElementById("progress").innerHTML = "";
}

function startUpload(me) {
	if( document.getElementById("uploader").files.length != 0 ){
		// sets an unique 'uploadid' field composed by session cookie and current time
		var session = document.getElementById('session').value;
		var time = Date.now();
		document.getElementById('submittime').value = time;
		
		// build upload URI adding the 'uploadid' variable
		me.action = document.getElementById('uploadurl').value + '?uploadid=' + session + time;
		
		// setup progress bar
		document.getElementById("progress").style.width = 0;
		document.getElementById("progress").innerHTML = "0%";
		toggleBarVisibility();
		
		// starts the delayed progress handler
		setTimeout("sendRequest()", 200);
		return true;
	}else{
		alert("You must choose a file !");
		return false;
	}
}*/


