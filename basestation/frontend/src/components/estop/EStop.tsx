import styles from './EStop.module.scss';

function EStop() {
	// TODO: not connected yet. This must send the stop command to the rover.
	function handleStop() {
		console.warn('E-stop pressed, but it is not connected to the rover');
	}

	return (
		<button
			className={styles.estop}
			title="Not connected to the rover yet"
			onClick={handleStop}
		>
			E-STOP
		</button>
	);
}

export default EStop;
