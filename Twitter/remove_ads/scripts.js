let chunks = [];
let source = Array.from(document.querySelectorAll('input[type="checkbox"]:checked'));

while (source.length) {
	chunks.push(source.splice(0, 20));
}

const event = new MouseEvent('click', {

	view: window,
	bubbles: true,
	cancelable: true
});

let minutes = 10; // amount of time to wait between batches
let interval = setInterval(() => {

	if (chunks.length === 0) {

		clearInterval(interval);
		return;
	}

	chunks.splice(0, 1)[0].forEach(i => {
		i.dispatchEvent(event);
	});

}, minutes * 60 * 1000);

