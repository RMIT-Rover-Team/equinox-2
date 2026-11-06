import EStop from '../estop/EStop';
import styles from './Banner.module.scss';

export type InfoBox = {
	label: string;
	value: string;
};

type BannerProps = {
	pageName: string;
	stationNumber: number;
	infoBoxes: InfoBox[];
};

function Banner({ pageName, stationNumber, infoBoxes }: BannerProps) {
	return (
		<header className={styles.banner}>
			<div
				className={styles.title}
				style={{ width: '35%' }}
			>
				<div className={styles.marker}></div>
				<div>
					<h1
						className={styles.pageName}
						style={{ textTransform: 'uppercase' }}
					>
						RMIT ROVER TEAM - {pageName}
					</h1>
					<p className={styles.station}>OPERATOR STATION {stationNumber}</p>
				</div>
			</div>

			<div className={styles.infoBoxes}>
				{infoBoxes.map(({ label, value }) => (
					<div
						key={label}
						className={styles.infoBox}
					>
						<p className={styles.infoLabel}>{label}</p>
						<p className={styles.infoValue}>{value}</p>
					</div>
				))}
			</div>

			<EStop />
		</header>
	);
}

export default Banner;
