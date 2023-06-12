// const host = "http://academia.go.ro:3024"
const host = "http://localhost:3024"


function getPhoneId() {
    let id = localStorage.getItem("sparkID");
    if (id == null) {
        id = "SPARK" + Math.floor(Math.random() * 100000000);
        localStorage.setItem("sparkID", id);
    }
    console.log("id: ", id);
    return id;
}

function getPhoneNr() {
    let inp = document.getElementById("phoneNumberInp");
    return inp.textContent;
}

function statuss(cb){
    fetch(host + "/status", {
        method: "GET",
        "Content-type": "application/json; charset=UTF-8"
    } 
    ).then((response) => { 
        response.json()
            .then(data => {
                cb(data);
            })
    }).catch(console.log)
}

function sendPark() {
    fetch(host+"/register", {
        method: "POST",
        body: JSON.stringify({
            id: getPhoneId(),
            phone: getPhoneNr()
        }),
        headers: {
            "Content-type": "application/json; charset=UTF-8"
        }
    }).then(response => {
        console.log("Server: ", response.data)
    }).catch(console.log)

}

setInterval(function(){
    statuss((data=>{
        console.log(data);
        let p = document.getElementById("avSpots");
        text = JSON.stringify(data);
        text = text.replace('{', '').replace('}', '').replaceAll('"', '');
        p.innerText = text;
    }));
}, 1000);

function change() {
    let elem = document.getElementById("submitBut");
    let toast = new bootstrap.Toast(document.getElementById("toastt"));
    let toasthtml = document.getElementById("toastt");
    if (elem.innerHTML == "Park") {
        sendPark();
        elem.innerHTML = "Retrieve";
        toast.show();
        toasthtml.innerHTML = "Vehicle has been parked."
    }
    else {
        sendPark();
        elem.innerHTML = "Park";
        toast.show();
        toasthtml.innerHTML = "Vehicle has been retrieved."
    }
}

function enableDisableBut() {
    let but = document.getElementById("submitBut");
    let input = document.getElementById("phoneNumberInp").value;
    if (input.length >= 10)
        but.classList.remove('disabled');
    else if (input.length < 10)
        but.classList.add('disabled');
}