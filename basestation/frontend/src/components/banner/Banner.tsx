import EStop from '../estop/EStop';
import styles from './Banner.module.scss';

export type InfoBox = {
	label: string;
	value: string;
};

type BannerProps = {
	pageName: string;
	infoBoxes: InfoBox[];
};

function Banner({ pageName, infoBoxes }: BannerProps) {
	return (
		<header className={styles.banner}>
			<div
				className={styles.title}
			>
				<div className={styles.marker}></div>
				<div>
					<h1 className={styles.pageName} >
						RMIT ROVER TEAM - {pageName}
					</h1>
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
