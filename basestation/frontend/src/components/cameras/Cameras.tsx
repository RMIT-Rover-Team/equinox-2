import { useState } from 'react';
import { useTracks, VideoTrack } from '@livekit/components-react';
import { Track } from 'livekit-client';

import styles from './Cameras.module.scss';

const SLOT_COUNT = 4;

function Cameras() {
	// Sorted so each <video> keeps its place in the page; moving it would pause the stream
	const tracks = useTracks([Track.Source.Camera, Track.Source.Unknown])
		.filter((track) => track.publication.kind === Track.Kind.Video)
		.sort((a, b) =>
			a.publication.trackSid.localeCompare(b.publication.trackSid),
		);
	const ids = tracks.map((track) => track.publication.trackSid);

	// order[0] is the large panel, order[1..3] are the small ones
	const [order, setOrder] = useState<string[]>([]);
	const slots = [
		...order.filter((id) => ids.includes(id)),
		...ids.filter((id) => !order.includes(id)),
	].slice(0, SLOT_COUNT);

	function makeMain(slot: number) {
		const next = [...slots];
		[next[0], next[slot]] = [next[slot], next[0]];
		setOrder(next);
	}

	function placement(slot: number) {
		return slot === 0
			? { gridRow: 1, gridColumn: '1 / -1' }
			: { gridRow: 2, gridColumn: slot };
	}

	return (
		<div className={styles.grid}>
			{tracks.map((track, index) => {
				const slot = slots.indexOf(track.publication.trackSid);
				if (slot === -1) return null;

				return (
					<button
						key={track.publication.trackSid}
						className={styles.feed}
						style={placement(slot)}
						disabled={slot === 0}
						onClick={() => makeMain(slot)}
					>
						<VideoTrack trackRef={track} />
						<span className={styles.label}>Camera {index + 1}</span>
					</button>
				);
			})}
			{Array.from({ length: SLOT_COUNT - slots.length }, (_, i) => (
				<div
					key={`empty-${i}`}
					className={styles.feed}
					style={placement(slots.length + i)}
				>
					<span className={styles.label}>No signal</span>
				</div>
			))}
		</div>
	);
}

export default Cameras;
