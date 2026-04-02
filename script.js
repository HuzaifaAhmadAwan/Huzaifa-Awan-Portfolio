// ===== PORTFOLIO INTERACTIVE SCRIPT =====

document.addEventListener('DOMContentLoaded', () => {
    setupWelcomeScreen();
    setupPortfolioOnEnter();
});

// ===== WELCOME SCREEN SETUP =====
function setupWelcomeScreen() {
    const welcomeScreen = document.getElementById('welcomeScreen');
    const introScreen = document.getElementById('introScreen');
    const bannerName = document.getElementById('bannerName');
    const backBtn = document.getElementById('backBtn');
    const continueBtn = document.getElementById('continueBtn');
    const portfolioMain = document.getElementById('portfolioMain');
    
    // Click on banner name to show introduction
    bannerName.addEventListener('click', showIntroScreen);
    
    // Back button to return to welcome screen
    if (backBtn) {
        backBtn.addEventListener('click', backToWelcome);
    }
    
    // Continue button to enter portfolio
    if (continueBtn) {
        continueBtn.addEventListener('click', enterPortfolio);
    }
    
    const CLOSE_DURATION = 5000;  // 5 seconds - hydraulic panel closing
    const OPENING_DURATION = 2500; // 2.5 seconds - dramatic panel opening
    
    // Animate progress counter from 0% to 100%
    function animateProgressCounter(duration) {
        const progressTexts = document.querySelectorAll('.progress-text');
        const startTime = Date.now();
        
        const updateProgress = () => {
            const elapsed = Date.now() - startTime;
            const percent = Math.min(Math.round((elapsed / duration) * 100), 100);
            progressTexts.forEach(el => {
                el.textContent = percent + '%';
            });
            if (percent < 100) {
                requestAnimationFrame(updateProgress);
            }
        };
        updateProgress();
    }
    
    function showIntroScreen() {
        // Get DOM elements
        const scrollLeft = document.getElementById('scrollLeft');
        const scrollRight = document.getElementById('scrollRight');
        const scrollGate = document.querySelector('.scroll-gate');
        const bannerIntro = document.querySelector('.banner-intro');
        const bannerNameEl = document.querySelector('.banner-name');
        const htmlEl = document.documentElement;
        
        // Display scroll gate and panels
        scrollGate.style.display = 'block';
        scrollLeft.style.display = 'block';
        scrollRight.style.display = 'block';
        
        // Add rolling-back class to trigger banner text roll animation
        bannerIntro.classList.add('rolling-back');
        bannerNameEl.classList.add('rolling-back');
        
        // Update HUD status labels
        const leftStatusLabel = scrollLeft.querySelector('.status-label');
        const rightStatusLabel = scrollRight.querySelector('.status-label');
        if (leftStatusLabel) leftStatusLabel.textContent = 'SEALING';
        if (rightStatusLabel) rightStatusLabel.textContent = 'LOCKING';
        
        // Start panel closing animation with proper classes
        scrollGate.classList.add('closing');
        scrollLeft.classList.add('closing');
        scrollRight.classList.add('closing');
        
        // Apply screen shake animation to html element
        htmlEl.classList.add('machinery-impact');
        
        // Animate the progress counter
        animateProgressCounter(CLOSE_DURATION);
        
        // Update status to LOCKED when panels collide
        setTimeout(() => {
            if (leftStatusLabel) leftStatusLabel.textContent = 'LOCKED';
            if (rightStatusLabel) rightStatusLabel.textContent = 'SEALED';
        }, CLOSE_DURATION * 0.61);
        
        // Swap screens when panels are fully closed
        setTimeout(() => {
            // Hide welcome screen
            welcomeScreen.style.display = 'none';
            
            // Show intro screen
            introScreen.style.display = 'flex';
            introScreen.classList.add('paper-unroll');
            
            // Set intro text
            document.getElementById('introText').textContent = portfolioData.bio;
        }, CLOSE_DURATION);
        
        // Trigger panel opening animation
        setTimeout(() => {
            scrollLeft.classList.remove('closing');
            scrollRight.classList.remove('closing');
            scrollLeft.classList.add('opening');
            scrollRight.classList.add('opening');
        }, CLOSE_DURATION);
        
        // Clear animation classes and hide panels
        setTimeout(() => {
            scrollGate.classList.remove('closing');
            scrollLeft.classList.remove('closing', 'opening');
            scrollRight.classList.remove('closing', 'opening');
            scrollGate.style.display = 'none';
            scrollLeft.style.display = 'none';
            scrollRight.style.display = 'none';
            htmlEl.classList.remove('machinery-impact');
        }, CLOSE_DURATION + OPENING_DURATION);
    }
    
    function backToWelcome() {
        // Get DOM elements
        const scrollLeft = document.getElementById('scrollLeft');
        const scrollRight = document.getElementById('scrollRight');
        const scrollGate = document.querySelector('.scroll-gate');
        const bannerIntro = document.querySelector('.banner-intro');
        const bannerNameEl = document.querySelector('.banner-name');
        const htmlEl = document.documentElement;
        
        // Display scroll gate and panels
        scrollGate.style.display = 'block';
        scrollLeft.style.display = 'block';
        scrollRight.style.display = 'block';
        
        // Update HUD status labels
        const leftStatusLabel = scrollLeft.querySelector('.status-label');
        const rightStatusLabel = scrollRight.querySelector('.status-label');
        if (leftStatusLabel) leftStatusLabel.textContent = 'OPENING';
        if (rightStatusLabel) rightStatusLabel.textContent = 'OPENING';
        
        // Start panel closing animation
        scrollGate.classList.add('closing');
        scrollLeft.classList.add('closing');
        scrollRight.classList.add('closing');
        
        // Apply screen shake effect
        htmlEl.classList.add('machinery-impact');
        
        // Animate the progress counter
        animateProgressCounter(CLOSE_DURATION);
        
        // Update status to LOCKED when panels collide
        setTimeout(() => {
            if (leftStatusLabel) leftStatusLabel.textContent = 'LOCKED';
            if (rightStatusLabel) rightStatusLabel.textContent = 'SEALED';
        }, CLOSE_DURATION * 0.61);
        
        // Reset banner animations
        bannerIntro.classList.remove('rolling-back');
        bannerNameEl.classList.remove('rolling-back');
        
        // Swap screens when panels are fully closed
        setTimeout(() => {
            welcomeScreen.style.display = 'flex';
            introScreen.style.display = 'none';
            introScreen.classList.remove('paper-unroll');
        }, CLOSE_DURATION);
        
        // Trigger panel opening animation
        setTimeout(() => {
            scrollLeft.classList.remove('closing');
            scrollRight.classList.remove('closing');
            scrollLeft.classList.add('opening');
            scrollRight.classList.add('opening');
        }, CLOSE_DURATION);
        
        // Clear animation classes and hide panels
        setTimeout(() => {
            scrollGate.classList.remove('closing');
            scrollLeft.classList.remove('closing', 'opening');
            scrollRight.classList.remove('closing', 'opening');
            scrollGate.style.display = 'none';
            scrollLeft.style.display = 'none';
            scrollRight.style.display = 'none';
            htmlEl.classList.remove('machinery-impact');
        }, CLOSE_DURATION + OPENING_DURATION);
        
        // Re-trigger banner unroll animation
        bannerIntro.style.animation = 'none';
        bannerNameEl.style.animation = 'none';
        setTimeout(() => {
            bannerIntro.style.animation = '';
            bannerNameEl.style.animation = '';
        }, 10);
    }
    
    function enterPortfolio() {
        // Get DOM elements
        const scrollLeft = document.getElementById('scrollLeft');
        const scrollRight = document.getElementById('scrollRight');
        const scrollGate = document.querySelector('.scroll-gate');
        const htmlEl = document.documentElement;
        
        // Display scroll gate and panels
        scrollGate.style.display = 'block';
        scrollLeft.style.display = 'block';
        scrollRight.style.display = 'block';
        
        // Update HUD status labels
        const leftStatusLabel = scrollLeft.querySelector('.status-label');
        const rightStatusLabel = scrollRight.querySelector('.status-label');
        if (leftStatusLabel) leftStatusLabel.textContent = 'PROCESSING';
        if (rightStatusLabel) rightStatusLabel.textContent = 'TRANSFER';
        
        // Start panel closing animation
        scrollGate.classList.add('closing');
        scrollLeft.classList.add('closing');
        scrollRight.classList.add('closing');
        
        // Apply screen shake effect
        htmlEl.classList.add('machinery-impact');
        
        // Animate the progress counter
        animateProgressCounter(CLOSE_DURATION);
        
        // Update status to LOCKED when panels collide
        setTimeout(() => {
            if (leftStatusLabel) leftStatusLabel.textContent = 'SEALED';
            if (rightStatusLabel) rightStatusLabel.textContent = 'SECURED';
        }, CLOSE_DURATION * 0.61);
        
        // Swap screens when panels are fully closed
        setTimeout(() => {
            introScreen.style.display = 'none';
            introScreen.classList.remove('paper-unroll');
            portfolioMain.style.display = 'block';
            portfolioMain.style.opacity = '0';
            
            // Initialize portfolio content
            populatePortfolio();
            setupPortfolioEventListeners();
            generateStars();
            document.getElementById('year').textContent = new Date().getFullYear();
        }, CLOSE_DURATION);
        
        // Trigger panel opening animation
        setTimeout(() => {
            scrollLeft.classList.remove('closing');
            scrollRight.classList.remove('closing');
            scrollLeft.classList.add('opening');
            scrollRight.classList.add('opening');
        }, CLOSE_DURATION);
        
        // Clear animation classes and hide panels
        setTimeout(() => {
            scrollGate.classList.remove('closing');
            scrollLeft.classList.remove('closing', 'opening');
            scrollRight.classList.remove('closing', 'opening');
            scrollGate.style.display = 'none';
            scrollLeft.style.display = 'none';
            scrollRight.style.display = 'none';
            htmlEl.classList.remove('machinery-impact');
            
            // Fade in portfolio content
            portfolioMain.style.transition = 'opacity 0.6s ease';
            portfolioMain.style.opacity = '1';
        }, CLOSE_DURATION + OPENING_DURATION);
    }
}

// ===== SETUP PORTFOLIO AFTER ENTRANCE =====
function setupPortfolioOnEnter() {
    // This will be called after the entrance animation
}

// ===== Populate Portfolio Data =====
function populatePortfolio() {
    // About Section
    document.getElementById('aboutText').textContent = portfolioData.bio;
    
    // Skills
    const skillsGrid = document.getElementById('skillsGrid');
    skillsGrid.innerHTML = '';
    portfolioData.skills.forEach(skill => {
        const skillTag = document.createElement('div');
        skillTag.className = 'skill-tag';
        skillTag.textContent = skill;
        skillsGrid.appendChild(skillTag);
    });
    
    // Projects
    populateProjects();
    
    // Experience
    populateExperience();
    
    // Education
    populateEducation();
    
    // Contact Information
    populateContact();
    
    // Social Links
    populateSocial();
    
    // CV Download Link
    if (portfolioData.cvUrl !== '#') {
        document.getElementById('cvDownload').href = portfolioData.cvUrl;
    }
}

function populateProjects() {
    const projectsGrid = document.getElementById('projectsGrid');
    projectsGrid.innerHTML = '';
    
    portfolioData.projects.forEach(project => {
        const projectCard = document.createElement('div');
        projectCard.className = 'project-card';
        projectCard.dataset.category = project.category;
        
        projectCard.innerHTML = `
            <div class="project-image">${project.image}</div>
            <div class="project-content">
                <h3>${project.title}</h3>
                <p>${project.description}</p>
                <div class="project-tags">
                    ${project.tags.map(tag => `<span class="project-tag">${tag}</span>`).join('')}
                </div>
                <a href="#" class="project-link" data-project-id="${project.id}">View Details →</a>
            </div>
        `;
        
        projectCard.querySelector('.project-link').addEventListener('click', (e) => {
            e.preventDefault();
            openProjectModal(project);
        });
        
        projectsGrid.appendChild(projectCard);
    });
}

function populateExperience() {
    const timeline = document.getElementById('experienceTimeline');
    timeline.innerHTML = '';
    
    portfolioData.experience.forEach(exp => {
        const timelineItem = document.createElement('div');
        timelineItem.className = 'timeline-item';
        timelineItem.innerHTML = `
            <h4>${exp.position}</h4>
            <div class="timeline-date">${exp.company} • ${exp.duration}</div>
            <div class="timeline-description">${exp.description}</div>
            ${exp.highlights ? `<ul class="highlights">${exp.highlights.map(h => `<li>${h}</li>`).join('')}</ul>` : ''}
        `;
        timeline.appendChild(timelineItem);
    });
}

function populateEducation() {
    const timeline = document.getElementById('educationTimeline');
    timeline.innerHTML = '';
    
    portfolioData.education.forEach(edu => {
        const timelineItem = document.createElement('div');
        timelineItem.className = 'timeline-item';
        timelineItem.innerHTML = `
            <h4>${edu.degree}</h4>
            <div class="timeline-date">${edu.institution} • ${edu.year}</div>
            <div class="timeline-description">${edu.field}</div>
            ${edu.details ? `<p style="margin-top: 0.5rem; color: var(--text-light);">${edu.details}</p>` : ''}
        `;
        timeline.appendChild(timelineItem);
    });
}

function populateContact() {
    const contactLinks = document.getElementById('contactLinks');
    contactLinks.innerHTML = '';
    
    const contactInfo = [
        { label: 'Email', value: portfolioData.contact.email, icon: 'fas fa-envelope' },
        { label: 'Phone', value: portfolioData.contact.phone, icon: 'fas fa-phone' },
        { label: 'Location', value: portfolioData.contact.location, icon: 'fas fa-map-marker-alt' }
    ];
    
    contactInfo.forEach(info => {
        if (info.value && info.value !== '#') {
            const link = document.createElement('a');
            link.className = 'contact-link';
            link.href = info.label === 'Email' ? `mailto:${info.value}` : '#';
            link.innerHTML = `<i class="${info.icon}"></i><span>${info.value}</span>`;
            contactLinks.appendChild(link);
        }
    });
}

function populateSocial() {
    const socialLinks = document.getElementById('socialLinks');
    socialLinks.innerHTML = '';
    
    portfolioData.social.forEach(social => {
        if (social.url !== '#') {
            const link = document.createElement('a');
            link.className = 'social-link';
            link.href = social.url;
            link.target = '_blank';
            link.title = social.name;
            link.innerHTML = `<i class="${social.icon}"></i>`;
            socialLinks.appendChild(link);
        }
    });
}

// ===== Event Listeners =====
function setupPortfolioEventListeners() {
    // Mobile Menu Toggle
    const hamburger = document.getElementById('hamburger');
    const navMenu = document.getElementById('navMenu');
    
    if (hamburger) {
        hamburger.addEventListener('click', () => {
            hamburger.classList.toggle('active');
            navMenu.classList.toggle('active');
        });
        
        // Close mobile menu on link click
        document.querySelectorAll('.nav-menu a').forEach(link => {
            link.addEventListener('click', () => {
                hamburger.classList.remove('active');
                navMenu.classList.remove('active');
            });
        });
    }
    
    // Filter Projects
    const filterButtons = document.querySelectorAll('.filter-btn');
    const projectCards = document.querySelectorAll('.project-card');
    
    filterButtons.forEach(button => {
        button.addEventListener('click', () => {
            filterButtons.forEach(btn => btn.classList.remove('active'));
            button.classList.add('active');
            
            const filter = button.dataset.filter;
            projectCards.forEach(card => {
                if (filter === 'all' || card.dataset.category === filter) {
                    card.style.display = 'block';
                    setTimeout(() => card.classList.add('fade-in-up'), 10);
                } else {
                    card.style.display = 'none';
                }
            });
        });
    });
    
    // Modal Handling
    const modal = document.getElementById('projectModal');
    const closeBtn = document.querySelector('.close');
    
    if (closeBtn) {
        closeBtn.addEventListener('click', () => {
            modal.style.display = 'none';
        });
        
        window.addEventListener('click', (e) => {
            if (e.target === modal) {
                modal.style.display = 'none';
            }
        });
    }
    
    // Contact Form
    const contactForm = document.getElementById('contactForm');
    if (contactForm) {
        contactForm.addEventListener('submit', handleContactSubmit);
    }
    
    // Smooth scrolling for anchor links
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            const href = this.getAttribute('href');
            if (href !== '#' && document.querySelector(href)) {
                e.preventDefault();
                const target = document.querySelector(href);
                target.scrollIntoView({ behavior: 'smooth' });
            }
        });
    });
}

// ===== Modal Functions =====
function openProjectModal(project) {
    const modal = document.getElementById('projectModal');
    const modalBody = document.getElementById('modalBody');
    
    modalBody.innerHTML = `
        <h2>${project.title}</h2>
        <div style="font-size: 3rem; margin: 1rem 0;">${project.image}</div>
        <p>${project.details}</p>
        <div class="project-tags">
            ${project.tags.map(tag => `<span class="project-tag">${tag}</span>`).join('')}
        </div>
        ${project.link !== '#' ? `<a href="${project.link}" class="btn btn-primary" target="_blank">View Project</a>` : ''}
    `;
    
    modal.style.display = 'block';
}

// ===== Contact Form Handler =====
function handleContactSubmit(e) {
    e.preventDefault();
    
    const inputs = e.target.querySelectorAll('input, textarea');
    const name = inputs[0].value;
    const email = inputs[1].value;
    const subject = inputs[2].value;
    const message = inputs[3].value;
    
    // Create mailto link
    const mailtoLink = `mailto:${portfolioData.contact.email}?subject=${encodeURIComponent(subject)}&body=${encodeURIComponent(`Name: ${name}\nEmail: ${email}\n\n${message}`)}`;
    
    // Open email client
    window.location.href = mailtoLink;
    
    // Show success message
    alert('Thank you for your message! Your email client will open momentarily.');
    e.target.reset();
}

// ===== Utility Functions =====
function generateStars() {
    const starsContainer = document.querySelector('.stars');
    if (!starsContainer) return;
    
    const starCount = 50;
    
    for (let i = 0; i < starCount; i++) {
        const star = document.createElement('div');
        star.className = 'star';
        star.style.left = Math.random() * 100 + '%';
        star.style.top = Math.random() * 100 + '%';
        star.style.animationDelay = Math.random() * 3 + 's';
        starsContainer.appendChild(star);
    }
}

// ===== Intersection Observer for Animations =====
const observerOptions = {
    threshold: 0.1,
    rootMargin: '0px 0px -100px 0px'
};

const observer = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
        if (entry.isIntersecting) {
            entry.target.classList.add('fade-in-up');
        }
    });
}, observerOptions);

// ===== Add Animation Classes =====
const style = document.createElement('style');
style.textContent = `
    .project-card {
        opacity: 0;
        animation: fadeInUp 0.6s ease forwards;
    }
`;
document.head.appendChild(style);

console.log("Portfolio loaded successfully!");
