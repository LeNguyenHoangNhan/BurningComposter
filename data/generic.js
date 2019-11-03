function setActiveNavItem() {
    var elems = document.getElementsByClassName("nav_bar_item");
    if (elems.length) {
        var activated_elems = document.getElementsByClassName("active");
        if (activated_elems.length) {
            for (var i = 0; i < activated_elems.length; i++) {
                activated_elems[i].classList.remove("active");
            }
        }
        if (window.location.pathname == "/" | window.location.pathname == "/index.html") {
            document.getElementsByClassName("nav_bar_home")[0].classList.add("active");
        } else if (window.location.pathname == "/monitor.html" | window.location.pathname == "/monitor") {
            document.getElementsByClassName("nav_bar_monitor")[0].classList.add("active");
        } else if (window.location.pathname == "/wificonfig.html" | window.location.pathname == "/wificonfig") {
            document.getElementsByClassName("nav_bar_config")[0].classList.add("active");
        } else if (window.location.pathname == "/about.html" | window.location.pathname == "/about") {
            document.getElementsByClassName("nav_bar_about")[0].classList.add("active");
        }
    }
}