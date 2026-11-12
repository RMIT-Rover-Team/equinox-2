import { useState } from 'react';

import styles from './Cameras.module.scss';

const cameras = ['Camera 1', 'Camera 2', 'Camera 3', 'Camera 4'];

function Cameras() {
	// order[0] is the large panel, order[1..3] are the small ones
	const [order, setOrder] = useState(cameras);

	function makeMain(slot: number) {
		const next = [...order];
		[next[0], next[slot]] = [next[slot], next[0]];
		setOrder(next);
	}

	return (
		<div className={styles.grid}>
			{cameras.map((camera) => {
				const slot = order.indexOf(camera);
				const isMain = slot === 0;

				return (
					<button
						key={camera}
						className={styles.feed}
						style={
							isMain
								? { gridRow: 1, gridColumn: '1 / -1' }
								: { gridRow: 2, gridColumn: slot }
						}
						disabled={isMain}
						onClick={() => makeMain(slot)}
					>
						{/* video goes here */}
						<span className={styles.label}>{camera}</span>
					</button>
				);
			})}
		</div>
	);
}

export default Cameras;
