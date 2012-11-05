/**
 * @author	<bensonzhang@gmail.com>
 * @date	2012-11-03
 */

function log(msg) {
	var now = new Date();
	$('#log').append('[' + now.toLocaleString() + '] ' + msg + '<br/>');
}

$(document).ready(function(){
    $('.prob').hide();
	$('.word').click(function() {
        $(this).next().toggle();
	});
});
