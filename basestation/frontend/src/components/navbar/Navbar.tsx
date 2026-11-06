import styles from './Navbar.module.scss';

const pages = ['Drive', 'Arm', 'Science', 'Excavator'] as const;

export type Page = (typeof pages)[number];

type NavbarProps = {
	page: Page;
	onPageChange: (page: Page) => void;
};

function Navbar({ page, onPageChange }: NavbarProps) {
	return (
		<nav className={styles.navbar}>
			{pages.map((name) => (
				<button
					key={name}
					className={styles.button}
					aria-current={name === page ? 'page' : undefined}
					onClick={() => onPageChange(name)}
				>
					{name}
				</button>
			))}
		</nav>
	);
}

export default Navbar;
