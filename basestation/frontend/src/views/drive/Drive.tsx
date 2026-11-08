import Cameras from '../../components/cameras/Cameras'
import IMU from './IMU'
import styles from './Drive.module.scss'

function Drive() {
	return (
		<div className={styles.body}>
			<section className={styles.panel}>
				<Cameras />
			</section>
      <section className={styles.panel}>
        <IMU/>
			</section>
		</div>
	);
}

export default Drive
